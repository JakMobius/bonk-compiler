
#include "x86_backend_context.hpp"
#include <iostream>
#include "command_dumper.hpp"
#include "x86_final_optimizer.hpp"

namespace bonk::x86_backend {

const MachineRegister SYSTEM_V_ARGUMENT_REGISTERS[] = {rdi, rsi, rdx, rcx, r8, r9};
const int SYSTEM_V_ARGUMENT_REGISTERS_COUNT =
    sizeof(SYSTEM_V_ARGUMENT_REGISTERS) / sizeof(MachineRegister);

const MachineRegister SYSTEM_V_CALLEE_PRESERVED_REGISTERS[] = {
    // rbp - we preserve it explicitly
    rbx, r12, r13, r14, r15};
const int SYSTEM_V_CALLEE_PRESERVED_REGISTERS_COUNT =
    sizeof(SYSTEM_V_CALLEE_PRESERVED_REGISTERS) / sizeof(MachineRegister);

const MachineRegister SYSTEM_V_CALLER_PRESERVED_REGISTERS[] = {rax, rcx, rdx, rsi, rdi,
                                                               r8,  r9,  r10, r11};
const int SYSTEM_V_CALLER_PRESERVED_REGISTERS_COUNT =
    sizeof(SYSTEM_V_CALLER_PRESERVED_REGISTERS) / sizeof(MachineRegister);

BackendContext::BackendContext(Compiler* compiler, macho::MachoFile* destination) {
    linked_compiler = compiler;
    target = destination;

    // TODO: better naming
    scope_stack = new ScopeStack();

    global_descriptors = new RegisterDescriptorList();
    current_descriptors = global_descriptors;
    procedure_return_register = -1;
    procedure_body_container = nullptr;
    procedure_parameters = 0;
}

BackendContext::~BackendContext() {
    delete scope_stack;
    delete global_descriptors;

    scope_stack = nullptr;
    global_descriptors = nullptr;

    current_descriptors = nullptr;
}

void BackendContext::compile_program(TreeNodeList* ast) {

    FieldList* scope = read_scope_variables(ast);

    if (!linked_compiler->state) {

        for (auto& next_node : ast->list) {
            if (next_node->type == TreeNodeType::n_block_definition) {
                write_block_definition((TreeNodeBlockDefinition*)next_node.get());
            } else if (next_node->type == TreeNodeType::n_var_definition) {
                write_global_var_definition((TreeNodeVariableDefinition*)next_node.get());
            }
        }

        int total_global_variables = 0;
        for (auto& next_node : ast->list) {
            if (next_node->type == TreeNodeType::n_var_definition) {
                auto* var_definition = (TreeNodeVariableDefinition*)next_node.get();
                if (var_definition->variable_value != nullptr) {
                    linked_compiler->error().at(var_definition->variable_value->source_position)
                        << "variables may not have initial value in x86 mode";
                }
                target->add_internal_symbol(
                    std::string(var_definition->variable_name->variable_name),
                    macho::SYMBOL_SECTION_DATA, total_global_variables * 8);

                // TODO: Rewrite add_data to optimize this
                std::vector<char> storage;
                for (int j = 0; j < 8; j++) {
                    storage.push_back(0);
                }
                target->add_data({storage.data(), storage.size()});
                total_global_variables++;
            }
        }

        for (auto var : scope->variables) {
            if (var->type == VARIABLE_TYPE_NUMBER) {
                auto descriptor = global_descriptors->get_descriptor(var->storage);
                descriptor->has_symbol_position = true;
                descriptor->located_in_symbol = true;
                descriptor->symbol_position =
                    target->get_symbol_from_name(std::string(var->identifier->variable_name));
            }
        }

        for (auto& next_node : ast->list) {
            if (next_node->type == TreeNodeType::n_block_definition) {
                auto* definition = (TreeNodeBlockDefinition*)next_node.get();
                write_block_implementation(definition);
            }
        }
    }

    if (scope) {
        scope_stack->pop_scope();
    }
}

Variable* BackendContext::field_list_declare_variable(TreeNodeVariableDefinition* node) {

    Variable* var = scope_stack->get_variable(node->variable_name.get(), nullptr);

    if (var == nullptr) {
        var = new VariableNumber(node);

        FieldList* top_scope = scope_stack->top();

        top_scope->add_variable(var);
        return var;
    } else {
        error_already_defined(node->variable_name.get());
    }

    return nullptr;
}

FieldList* BackendContext::read_scope_variables(TreeNodeList* node) {
    auto* scope = new FieldList(current_descriptors, state);
    scope_stack->push_scope(scope);

    for (auto& next_node : node->list) {
        TreeNodeType node_type = next_node->type;
        if (node_type == TreeNodeType::n_var_definition) {
            auto definition = (TreeNodeVariableDefinition*)next_node.get();
            auto new_variable = field_list_declare_variable(definition);
            if (new_variable != nullptr) {
                if (definition->is_contextual) {
                    locate_procedure_parameter(new_variable);
                }
            }
        } else if (node_type == TreeNodeType::n_block_definition) {
            field_list_declare_block((TreeNodeBlockDefinition*)next_node.get());
        }

        if (linked_compiler->state) {
            return nullptr;
        }
    }

    return scope;
}

FieldList* BackendContext::field_list_find_block_parameters(TreeNodeBlockDefinition* block) const {
    auto* argument_list = new FieldList(current_descriptors, state);

    for (auto& next_node : block->body->list) {
        TreeNodeType node_type = next_node->type;
        if (node_type != TreeNodeType::n_var_definition)
            break;

        auto* var_definition = (TreeNodeVariableDefinition*)next_node.get();

        if (!var_definition->is_contextual)
            break;

        Variable* number = new VariableNumber(var_definition);
        argument_list->add_variable(number);
    }

    return argument_list;
}

void BackendContext::field_list_declare_block(TreeNodeBlockDefinition* node) {
    auto* identifier = node->block_name.get();

    Variable* var = scope_stack->get_variable(identifier, nullptr);

    if (var == nullptr) {
        FieldList* argument_list = field_list_find_block_parameters(node);
        VariableFunction* func = nullptr;

        if (argument_list)
            func = new VariableFunction(identifier, argument_list);

        FieldList* top_scope = scope_stack->top();
        top_scope->add_variable((Variable*)func);
    } else {
        error_already_defined(identifier);
    }
}

void BackendContext::compile_block(TreeNodeList* block) {
    FieldList* scope = read_scope_variables(block);

    if (!linked_compiler->state) {
        for (auto& next_node : block->list) {
            compile_line(next_node.get());
        }
    }

    if (scope) {
        scope_stack->pop_scope();
    }
}

void BackendContext::preserve_callee_registers() const {
    std::vector<AbstractRegister> registers;

    for (auto i : SYSTEM_V_CALLEE_PRESERVED_REGISTERS) {
        registers.push_back(i);
    }

    state->current_command_list->commands.push_back(new RegPreserveCommand(registers));
}

void BackendContext::compile_block_definition(TreeNodeBlockDefinition* block) {

    procedure_parameters = 0;
    procedure_return_register = procedure_command_buffer->descriptors.next_constrained_register(
        rax, state->current_command_list);

    push_state();

    procedure_body_container = state->current_command_list;

    preserve_callee_registers();
    state->current_command_list->commands.push_back(new FrameCreateCommand());

    // As function body is a block which may use
    // caller-preserved registers, we should create
    // a compiler state.

    push_state();

    compile_block(block->body.get());

    pop_state();

    state->current_command_list->commands.push_back(new FrameDestroyCommand());
    state->current_command_list->commands.push_back(new RetCommand());
    preserve_callee_registers();

    pop_state();
}

void BackendContext::write_block_promise(TreeNodeBlockDefinition* block) {
}

void BackendContext::write_block_implementation(TreeNodeBlockDefinition* block) {
    procedure_command_buffer = new CommandBuffer(global_descriptors, target);
    current_descriptors = &procedure_command_buffer->descriptors;

    push_initial_state();
    compile_block_definition(block);
    pop_initial_state();

    CommandDumper().dump_list(procedure_command_buffer->root_list, StdOutputStream(std::cout), 0);

    auto colorized_buffer = RegisterColorizer::colorize(procedure_command_buffer);

    FinalOptimizer::optimize(colorized_buffer);
    if (linked_compiler->config.listing_file) {
        linked_compiler->config.listing_file.get_stream()
            << "; == function ==\n"
            << block->block_name->variable_name << ":\n";

        CommandDumper dumper;
        dumper.dump_list(colorized_buffer->root_list, linked_compiler->config.listing_file, 0);
    }
    colorized_buffer->write_block_to_object_file(std::string(block->block_name->variable_name),
                                                 target);

    delete colorized_buffer;
    delete procedure_command_buffer;
    current_descriptors = global_descriptors;
}

void BackendContext::compile_plus(TreeNodeOperator* expression) {
    if (expression->left)
        compile_expression(expression->left.get());
    else
        state->register_stack.push_imm64(0);
    compile_expression(expression->right.get());
    state->register_stack.add();
}

void BackendContext::compile_minus(TreeNodeOperator* expression) {
    if (expression->left)
        compile_expression(expression->left.get());
    else
        state->register_stack.push_imm64(0);
    compile_expression(expression->right.get());
    state->register_stack.sub();
}

void BackendContext::compile_multiply(TreeNodeOperator* expression) {
    compile_expression(expression->left.get());
    compile_expression(expression->right.get());
    state->register_stack.mul();
}

void BackendContext::compile_divide(TreeNodeOperator* expression) {
    compile_expression(expression->left.get());
    compile_expression(expression->right.get());
    state->register_stack.div();
}

void BackendContext::compile_line(TreeNode* node) {
    if (node->type == TreeNodeType::n_var_definition) {
        auto* var_definition = (TreeNodeVariableDefinition*)node;
        if (var_definition->variable_value)
            compile_assignment(var_definition->variable_name.get(),
                               var_definition->variable_value.get());
        else
            return;
    } else if (node->type == TreeNodeType::n_cycle) {
        compile_cycle((TreeNodeCycle*)node);
    } else if (node->type == TreeNodeType::n_check) {
        compile_check((TreeNodeCheck*)node);
    } else {
        compile_expression(node);
    }
}

void BackendContext::compile_expression(TreeNode* node) {
    if (node->type == TreeNodeType::n_operator) {
        auto* oper = (TreeNodeOperator*)node;

        switch (oper->oper_type) {
        case OperatorType::o_assign:
            compile_assignment((TreeNodeIdentifier*)oper->left.get(), oper->right.get());
            break;
        case OperatorType::o_plus:
            compile_plus(oper);
            break;
        case OperatorType::o_minus:
            compile_minus(oper);
            break;
        case OperatorType::o_multiply:
            compile_multiply(oper);
            break;
        case OperatorType::o_divide:
            compile_divide(oper);
            break;
        case OperatorType::o_or:
            compile_or(oper);
            break;
        case OperatorType::o_and:
            compile_and(oper);
            break;
        case OperatorType::o_equal:
            compile_equals(oper);
            break;
        case OperatorType::o_less:
            compile_less_than(oper);
            break;
        case OperatorType::o_less_equal:
            compile_less_or_equal_than(oper);
            break;
        case OperatorType::o_greater:
            compile_greater_than(oper);
            break;
        case OperatorType::o_greater_equal:
            compile_greater_or_equal_than(oper);
            break;
        case OperatorType::o_not_equal:
            compile_not_equal(oper);
            break;
        case OperatorType::o_brek:
            compile_brek_statement(oper);
            break;
        case OperatorType::o_bonk:
            compile_bonk_statement(oper);
            break;
            //            case OperatorType::o_PRINT:      compile_print           (oper); break; case
            //            OperatorType::o_BAMS:       compile_inline_assembly (oper); break; default:
            //            compile_math_comparsion (oper);
        default:
            assert(false);
            break;
        }
    } else if (node->type == TreeNodeType::n_call) {
        compile_call((TreeNodeCall*)node);
    } else if (node->type == TreeNodeType::n_number) {
        state->register_stack.push_imm64(((TreeNodeNumber*)node)->integer_value);
    } else if (node->type == TreeNodeType::n_identifier) {
        AbstractRegister reg = get_variable((TreeNodeIdentifier*)node);
        if (reg == -1)
            return;
        state->register_stack.push_reg64(reg);
    }
}

void BackendContext::compile_assignment(TreeNodeIdentifier* variable, TreeNode* value) {
    compile_expression(value);
    AbstractRegister reg = get_variable(variable);
    if (reg == -1)
        return;

    state->register_stack.write_head(reg);
}

AbstractRegister BackendContext::get_variable(TreeNodeIdentifier* identifier) {
    FieldList* scope = nullptr;
    Variable* variable = scope_stack->get_variable(identifier, &scope);

    if (variable == nullptr) {
        error_undefined_reference(identifier);
        return -1;
    }

    if (variable->type == VARIABLE_TYPE_FUNCTION) {
        linked_compiler->error().at(identifier->source_position)
            << identifier->variable_name << " is a block name, not variable";
        return -1;
    }

    return variable->storage;
}

void BackendContext::push_state() {
    auto new_command_list = procedure_command_buffer->next_command_list();
    state_stack.emplace_back(new_command_list);
    state = &state_stack[state_stack.size() - 1];
}

void BackendContext::pop_state() {
    auto previous_state = &state_stack[state_stack.size() - 2];
    auto* command = new ScopeCommand(state->current_command_list);
    state_stack.pop_back();
    state = previous_state;
    state->current_command_list->commands.push_back(command);
}

void BackendContext::push_initial_state() {
    state_stack.emplace_back(procedure_command_buffer->root_list);
    state = &state_stack[state_stack.size() - 1];
}

void BackendContext::pop_initial_state() {
    state_stack.pop_back();
    state = nullptr;
    procedure_body_container = nullptr;
}

void BackendContext::compile_cycle(TreeNodeCycle* cycle) {

    auto old_cycle_container = cycle_container;
    auto old_cycle_body = cycle_body;
    auto old_cycle_head = cycle_head;
    auto old_cycle_tail = cycle_tail;

    cycle_head = create_label();
    cycle_tail = create_label();

    cycle_container = state->current_command_list;
    state->current_command_list->commands.push_back(cycle_head);

    push_state();
    compile_block(cycle->body.get());
    state->current_command_list->commands.push_back(new ScopeRepeatCommand());
    pop_state();

    state->current_command_list->commands.push_back(new JumpCommand(cycle_head, COMMAND_JMP));
    state->current_command_list->commands.push_back(cycle_tail);

    cycle_container = old_cycle_container;
    cycle_body = old_cycle_body;
    cycle_head = old_cycle_head;
    cycle_tail = old_cycle_tail;
}

void BackendContext::error_already_defined(TreeNodeIdentifier* identifier) {
    linked_compiler->error().at(identifier->source_position)
        << "variable '" << identifier->variable_name << "' is already defined";
}

void BackendContext::error_undefined_reference(TreeNodeIdentifier* node) const {
    linked_compiler->error().at(node->source_position)
        << "undefined reference: '" << node->variable_name << "'";
}

void BackendContext::pop_to_scope(CommandList* scope) const {
    for (int i = state_stack.size() - 1, pop = 0;; i--, pop++) {
        if (scope == state_stack[i].current_command_list) {
            state->current_command_list->commands.push_back(new ScopePopCommand(pop));
            return;
        }
        assert(i != 0);
    }
}

void BackendContext::compile_bonk_statement(TreeNodeOperator* oper) {
    if (oper->right) {
        compile_expression(oper->right.get());
        state->register_stack.pop(procedure_return_register);
    } else {
        state->current_command_list->commands.push_back(
            new XorCommand(CommandParameter::create_register_64(procedure_return_register),
                           CommandParameter::create_register_64(procedure_return_register)));
    }

    pop_to_scope(procedure_body_container);

    state->current_command_list->commands.push_back(new FrameDestroyCommand());
    state->current_command_list->commands.push_back(new RetCommand(procedure_return_register));
    state->current_command_list->commands.push_back(new ScopeDeadEndCommand());
}

bool BackendContext::can_use_fast_logic(TreeNodeOperator* oper) {
    return can_use_fast_logic_on_operand(oper->left.get(), oper->oper_type) &&
           can_use_fast_logic_on_operand(oper->right.get(), oper->oper_type);
}

bool BackendContext::can_use_fast_logic_on_operand(TreeNode* operand, OperatorType oper_type) {
    if (operand->type == TreeNodeType::n_operator) {
        if (((TreeNodeOperator*)operand)->oper_type == oper_type) {
            return can_use_fast_logic((TreeNodeOperator*)operand);
        }
    }
    return operand->type == TreeNodeType::n_number || operand->type == TreeNodeType::n_identifier;
}

bool BackendContext::compile_logic_operand_recursive(TreeNode* node, JmpLabel* control_label,
                                                     CommandList* control_label_list,
                                                     OperatorType oper_type, bool check_for) {
    auto guard = compile_boolean_jump(node, check_for);
    if (!guard)
        return false;
    pop_to_scope(control_label_list);
    state->current_command_list->commands.push_back(new JumpCommand(control_label, COMMAND_JMP));
    guard->set_label(insert_label());
    return true;
}

bool BackendContext::compile_logic_recursive(TreeNodeOperator* oper, JmpLabel* control_label,
                                             CommandList* control_label_list,
                                             OperatorType oper_type, bool check_for) {
    if (oper->left->type == TreeNodeType::n_operator &&
        ((TreeNodeOperator*)oper->left.get())->oper_type == oper_type) {
        // left-hand operand must be expression
        bool is_expr = compile_logic_recursive((TreeNodeOperator*)(oper->left.get()), control_label,
                                               control_label_list, oper_type, check_for);
        assert(is_expr);
    } else {
        // left-hand operand must be expression
        bool is_expr = compile_logic_operand_recursive(oper->left.get(), control_label,
                                                       control_label_list, oper_type, check_for);
        assert(is_expr);
    }

    if (oper->right->type == TreeNodeType::n_operator &&
        ((TreeNodeOperator*)oper->right.get())->oper_type == oper_type) {
        return compile_logic_recursive((TreeNodeOperator*)(oper->right.get()), control_label,
                                       control_label_list, oper_type, check_for);
    } else {
        return compile_logic_operand_recursive(oper->right.get(), control_label, control_label_list,
                                               oper_type, check_for);
    }
}

void BackendContext::compile_jump_logical(TreeNodeOperator* oper) {
    bool is_and = oper->oper_type == OperatorType::o_and;
    auto control_label_list = state->current_command_list;
    auto control_label = create_label();
    auto flag = state->register_stack.push_placeholder(true);

    push_state();
    bool is_expression =
        compile_logic_recursive(oper, control_label, control_label_list, oper->oper_type, is_and);
    pop_state();

    if (is_expression) {
        auto end_label_list = state->current_command_list;
        auto end_label = create_label();
        push_state();
        state->current_command_list->commands.push_back(new MovCommand(
            CommandParameter::create_register_8(flag), CommandParameter::create_imm32(is_and)));
        pop_to_scope(end_label_list);
        state->current_command_list->commands.push_back(new JumpCommand(end_label, COMMAND_JMP));
        state->current_command_list->commands.push_back(new ScopeDeadEndCommand());
        pop_state();

        state->current_command_list->commands.push_back(control_label);

        push_state();
        state->current_command_list->commands.push_back(new MovCommand(
            CommandParameter::create_register_8(flag), CommandParameter::create_imm32(!is_and)));
        pop_state();

        state->current_command_list->commands.push_back(end_label);
    } else {
        state->current_command_list->commands.push_back(control_label);
    }
}

void BackendContext::compile_or(TreeNodeOperator* oper) {
    if (can_use_fast_logic(oper)) {
        compile_expression(oper->left.get());
        compile_expression(oper->right.get());
        state->register_stack.logical_or();
    } else {
        compile_jump_logical(oper);
    }
}

void BackendContext::compile_and(TreeNodeOperator* oper) {
    if (can_use_fast_logic(oper)) {
        compile_expression(oper->left.get());
        compile_expression(oper->right.get());
        state->register_stack.logical_and();
    } else {
        compile_jump_logical(oper);
    }
}

void BackendContext::compile_equals(TreeNodeOperator* oper) {
    compile_expression(oper->left.get());
    compile_expression(oper->right.get());
    state->register_stack.equals();
}

void BackendContext::compile_less_than(TreeNodeOperator* oper) {
    compile_expression(oper->left.get());
    compile_expression(oper->right.get());
    state->register_stack.less_than();
}

void BackendContext::compile_less_or_equal_than(TreeNodeOperator* oper) {
    compile_expression(oper->left.get());
    compile_expression(oper->right.get());
    state->register_stack.less_or_equal_than();
}

void BackendContext::compile_greater_than(TreeNodeOperator* oper) {
    compile_expression(oper->left.get());
    compile_expression(oper->right.get());
    state->register_stack.greater_than();
}

void BackendContext::compile_greater_or_equal_than(TreeNodeOperator* oper) {
    compile_expression(oper->left.get());
    compile_expression(oper->right.get());
    state->register_stack.greater_or_equal_than();
}

void BackendContext::compile_not_equal(TreeNodeOperator* oper) {
    compile_expression(oper->left.get());
    compile_expression(oper->right.get());
    state->register_stack.not_equal();
}

JumpCommand* BackendContext::append_oper_jmp_command(TreeNodeOperator* oper, bool inversed) {
    compile_expression(oper->left.get());
    compile_expression(oper->right.get());
    state->register_stack.compare();

    auto command = COMMAND_JMP;
    switch (oper->oper_type) {
    case OperatorType::o_equal:
        command = COMMAND_JE;
        break;
    case OperatorType::o_less:
        command = COMMAND_JL;
        break;
    case OperatorType::o_less_equal:
        command = COMMAND_JNG;
        break;
    case OperatorType::o_greater:
        command = COMMAND_JG;
        break;
    case OperatorType::o_greater_equal:
        command = COMMAND_JNL;
        break;
    case OperatorType::o_not_equal:
        command = COMMAND_JNE;
        break;
    default:
        assert(false);
    }

    auto jmp = new JumpCommand(create_label(), command);

    if (inversed) {
        jmp->invert_condition();
    }
    state->current_command_list->commands.push_back(jmp);
    return jmp;
}

JumpCommand* BackendContext::compile_boolean_jump(TreeNode* node, bool check_for) {
    JumpCommand* jmp = nullptr;

    if (node->type == TreeNodeType::n_operator) {
        auto oper = (TreeNodeOperator*)node;
        switch (oper->oper_type) {
        case OperatorType::o_equal:
        case OperatorType::o_less:
        case OperatorType::o_less_equal:
        case OperatorType::o_greater:
        case OperatorType::o_greater_equal:
        case OperatorType::o_not_equal:
            jmp = append_oper_jmp_command(oper, !check_for);
            break;
        default:
            break;
        }
    }

    if (!jmp) {
        long long old_size = state->register_stack.stack.size();
        compile_expression(node);
        if (state->register_stack.stack.size() == old_size)
            return nullptr;
        state->register_stack.test();
        if (check_for) {
            jmp = new JumpCommand(nullptr, COMMAND_JNE);
        } else {
            jmp = new JumpCommand(nullptr, COMMAND_JE);
        }
        state->current_command_list->commands.push_back(jmp);
    }

    return jmp;
}

// I'm proud of how natural this code look like
void BackendContext::compile_check(TreeNodeCheck* node) {

    auto jmp = compile_boolean_jump(node->condition.get(), false);

    push_state();
    compile_block(node->check_body.get());
    pop_state();

    jmp->set_label(insert_label());
}

void BackendContext::compile_brek_statement(TreeNodeOperator* oper) {
    if (!cycle_container) {
        linked_compiler->error().at(oper->source_position) << "brek operator outside of cycle";
        return;
    }

    pop_to_scope(cycle_container);
    state->current_command_list->commands.push_back(new JumpCommand(cycle_tail, COMMAND_JMP));
}

void BackendContext::compile_rebonk_statement(TreeNodeOperator* oper) {
    if (!cycle_body) {
        linked_compiler->error().at(oper->source_position) << "rebonk operator outside of cycle";
        return;
    }

    pop_to_scope(cycle_body);
    state->current_command_list->commands.push_back(new JumpCommand(cycle_head, COMMAND_JMP));
}

JmpLabel* BackendContext::create_label() const {
    return new JmpLabel(state->current_command_list->parent_buffer->labels++);
}

JmpLabel* BackendContext::insert_label() const {
    auto nop = new JmpLabel(state->current_command_list->parent_buffer->labels++);
    state->current_command_list->commands.push_back(nop);
    return nop;
}

TreeNode* BackendContext::call_argument_list_get_value(TreeNodeList* argument_list,
                                                       TreeNodeIdentifier* identifier) {
    if (!argument_list)
        return nullptr;

    for (auto& parameter : argument_list->list) {
        auto call_parameter = (TreeNodeCallParameter*)parameter.get();
        if (call_parameter->parameter_name->variable_name == identifier->variable_name) {
            return call_parameter->parameter_value.get();
        }
    }

    return nullptr;
}

TreeNode* BackendContext::compile_nth_argument(VariableFunction* func, TreeNodeList* argument_list,
                                               int i) {
    Variable* argument = func->argument_list->variables[i];

    TreeNode* value = call_argument_list_get_value(argument_list, argument->identifier);
    if (value == nullptr) {
        linked_compiler->error().at(func->identifier->source_position)
            << func->identifier->variable_name << " requires contextual variable "
            << argument->identifier->variable_name.size();
        return nullptr;
    }

    int stack_size = state->register_stack.stack.size();
    compile_expression(value);
    if (state->register_stack.stack.size() == stack_size) {
        linked_compiler->error().at(value->source_position)
            << "expression does not return anything";
    }

    return value;
}

void BackendContext::compile_call(TreeNodeCall* call) {
    auto* argument_list = call->call_parameters.get();
    auto* function_name = (TreeNodeIdentifier*)call->call_function.get();

    Variable* var = scope_stack->get_variable(function_name, nullptr);

    if (var == nullptr) {
        error_undefined_reference(function_name);
        return;
    }

    if (var->type != VARIABLE_TYPE_FUNCTION) {
        linked_compiler->error().at(function_name->source_position)
            << function_name->variable_name << " is not a function";
        return;
    }

    auto* func = (VariableFunction*)var;
    FieldList* function_arguments = func->argument_list;

    // rdi rsi rdx rcx r8 r9 stack

    int register_arguments = function_arguments->variables.size();
    if (register_arguments > SYSTEM_V_ARGUMENT_REGISTERS_COUNT) {
        register_arguments = SYSTEM_V_ARGUMENT_REGISTERS_COUNT;
    }
    int stack_arguments = function_arguments->variables.size() - register_arguments;

    state->current_command_list->commands.push_back(
        AlignStackCommand::create_before(stack_arguments));

    for (int i = function_arguments->variables.size() - 1; i >= SYSTEM_V_ARGUMENT_REGISTERS_COUNT;
         i--) {
        auto value = compile_nth_argument(func, argument_list, i);
        if (!value)
            return;

        AbstractRegister reg = state->register_stack.get_head_register_number();
        state->register_stack.decrease_stack_size();
        state->current_command_list->commands.push_back(new PushCommand(reg));
    }

    AbstractRegister arguments[SYSTEM_V_ARGUMENT_REGISTERS_COUNT] = {};

    for (int i = 0;
         i < SYSTEM_V_ARGUMENT_REGISTERS_COUNT && i < function_arguments->variables.size(); i++) {
        auto value = compile_nth_argument(func, argument_list, i);
        if (!value)
            return;

        AbstractRegister reg = procedure_command_buffer->descriptors.next_constrained_register(
            SYSTEM_V_ARGUMENT_REGISTERS[i], state->current_command_list);
        arguments[i] = reg;
        state->register_stack.pop(reg);
    }

    // Add a command to mark caller-preserved registers dirty

    std::vector<AbstractRegister> caller_preserved_registers;

    for (auto i : SYSTEM_V_CALLER_PRESERVED_REGISTERS) {
        auto reg = procedure_command_buffer->descriptors.next_constrained_register(
            i, state->current_command_list);
        caller_preserved_registers.push_back(reg);
    }

    state->current_command_list->commands.push_back(
        new RegPreserveCommand(caller_preserved_registers));

    // Now it's safe to call the function

    state->current_command_list->commands.push_back(new CallCommand(
        CommandParameterSymbol(
            true, target->get_symbol_from_name(std::string(call->call_function->variable_name))),
        arguments, register_arguments));

    state->current_command_list->commands.push_back(
        AlignStackCommand::create_after(stack_arguments));
    state->register_stack.push_reg64(
        procedure_command_buffer->descriptors.next_constrained_register(
            rax, state->current_command_list));
}

void BackendContext::write_block_definition(TreeNodeBlockDefinition* definition) const {
    target->declare_internal_symbol(std::string(definition->block_name->variable_name));
}

void BackendContext::write_global_var_definition(TreeNodeVariableDefinition* definition) const {
    target->declare_internal_symbol(std::string(definition->variable_name->variable_name));
}

void BackendContext::locate_procedure_parameter(Variable* parameter) {
    if (procedure_parameters < SYSTEM_V_ARGUMENT_REGISTERS_COUNT) {
        MachineRegister reg = SYSTEM_V_ARGUMENT_REGISTERS[procedure_parameters];
        state->current_command_list->commands.push_back(
            new LocateRegCommand(parameter->storage, CommandParameter::create_register_64(reg)));
    } else {
        int state_position = (procedure_parameters - SYSTEM_V_ARGUMENT_REGISTERS_COUNT + 2) * -8;
        state->current_command_list->commands.push_back(new LocateRegCommand(
            parameter->storage, CommandParameter::create_imm32(state_position)));
    }

    procedure_parameters++;
}

} // namespace bonk::x86_backend