
#include "x86_backend_context.hpp"
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

void BackendContext::compile_program(TreeNodeList<TreeNode*>* ast) {

    FieldList* scope = read_scope_variables(ast);

    if (!linked_compiler->state) {

        std::list<TreeNode*>* list = &ast->list;

        for (auto i = ast->list.begin(); i != ast->list.end(); ++i) {
            auto* next_node = *i;

            if (next_node->type == TREE_NODE_TYPE_BLOCK_DEFINITION) {
                write_block_definition((TreeNodeBlockDefinition*)next_node);
            } else if (next_node->type == TREE_NODE_TYPE_VAR_DEFINITION) {
                write_global_var_definition((TreeNodeVariableDefinition*)next_node);
            }
        }

        int total_global_variables = 0;
        for (auto i = list->begin(); i != list->end(); ++i) {
            auto* next_node = *i;

            if (next_node->type == TREE_NODE_TYPE_VAR_DEFINITION) {
                auto* var_definition = (TreeNodeVariableDefinition*)next_node;
                if (var_definition->variable_value != nullptr) {
                    linked_compiler->error_positioned(
                        var_definition->variable_value->source_position,
                        "variables may not have initial value in x86 mode");
                }
                target->add_internal_symbol(var_definition->variable_name->variable_name,
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

        for (int i = 0; i < scope->variables.size(); i++) {
            Variable* var = scope->variables[i];
            if (var->type == VARIABLE_TYPE_NUMBER) {
                auto descriptor = global_descriptors->get_descriptor(var->storage);
                descriptor->has_symbol_position = true;
                descriptor->located_in_symbol = true;
                descriptor->symbol_position =
                    target->get_symbol_from_name(var->identifier->variable_name);
            }
        }

        for (auto i = ast->list.begin(); i != ast->list.end(); ++i) {
            auto* next_node = *i;

            if (next_node->type == TREE_NODE_TYPE_BLOCK_DEFINITION) {
                auto* definition = (TreeNodeBlockDefinition*)next_node;
                if (definition->is_promise) {
                    write_block_promise(definition);
                } else {
                    write_block_implementation(definition);
                }
            }
        }
    }

    if (scope) {
        scope_stack->pop_scope();
    }
}

Variable* BackendContext::field_list_declare_variable(TreeNodeVariableDefinition* node) {

    Variable* var = scope_stack->get_variable(node->variable_name, nullptr);

    if (var == nullptr) {
        var = new VariableNumber(node);

        FieldList* top_scope = scope_stack->top();

        top_scope->add_variable(var);
        return var;
    } else {
        error_already_defined(node->variable_name);
    }

    return nullptr;
}

FieldList* BackendContext::read_scope_variables(TreeNodeList<TreeNode*>* node) {
    auto* scope = new FieldList(current_descriptors, state);
    scope_stack->push_scope(scope);

    std::list<TreeNode*>* list = &node->list;

    for (auto i = list->begin(); i != list->end(); ++i) {
        auto* next_node = *i;

        TreeNodeType node_type = next_node->type;
        if (node_type == TREE_NODE_TYPE_VAR_DEFINITION) {
            auto definition = (TreeNodeVariableDefinition*)next_node;
            auto new_variable = field_list_declare_variable(definition);
            if (new_variable != nullptr) {
                if (definition->is_contextual) {
                    locate_procedure_parameter(new_variable);
                }
            }
        } else if (node_type == TREE_NODE_TYPE_BLOCK_DEFINITION) {
            field_list_declare_block((TreeNodeBlockDefinition*)next_node);
        }

        if (linked_compiler->state) {
            return nullptr;
        }
    }

    return scope;
}

FieldList* BackendContext::field_list_find_block_parameters(TreeNodeBlockDefinition* block) const {
    auto* argument_list = new FieldList(current_descriptors, state);
    if (!argument_list)
        return nullptr;

    std::list<TreeNode*>* list = &block->body->list;

    for (auto i = list->begin(); i != list->end(); ++i) {
        auto* next_node = *i;

        TreeNodeType node_type = next_node->type;
        if (node_type != TREE_NODE_TYPE_VAR_DEFINITION)
            break;

        auto* var_definition = (TreeNodeVariableDefinition*)next_node;

        if (!var_definition->is_contextual)
            break;

        Variable* number = new VariableNumber(var_definition);

        if (!number || !argument_list->add_variable(number)) {
            linked_compiler->out_of_memory();
            delete argument_list;
            return nullptr;
        }
    }

    return argument_list;
}

void BackendContext::field_list_declare_block(TreeNodeBlockDefinition* node) {
    auto* identifier = node->block_name;

    Variable* var = scope_stack->get_variable(identifier, nullptr);

    if (var == nullptr) {
        FieldList* argument_list = field_list_find_block_parameters(node);
        VariableFunction* func = nullptr;

        if (argument_list)
            func = new VariableFunction(identifier, argument_list);

        FieldList* top_scope = scope_stack->top();

        if (!func || !top_scope->add_variable((Variable*)func))
            linked_compiler->out_of_memory();
    } else {
        error_already_defined(identifier);
    }
}

void BackendContext::compile_block(TreeNodeList<TreeNode*>* block) {
    FieldList* scope = read_scope_variables(block);

    if (!linked_compiler->state) {
        std::list<TreeNode*>* list = &block->list;

        for (auto i = list->begin(); i != list->end(); ++i) {
            auto* next_node = *i;
            compile_line(next_node);
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

    compile_block(block->body);

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

    CommandDumper().dump_list(procedure_command_buffer->root_list, stdout, 0);

    auto colorized_buffer = RegisterColorizer::colorize(procedure_command_buffer);

    FinalOptimizer::optimize(colorized_buffer);
    if (linked_compiler->config->listing_file) {
        fprintf(linked_compiler->config->listing_file, "; == function ==\n%s:\n",
                block->block_name->variable_name.c_str());

        CommandDumper dumper;
        dumper.dump_list(colorized_buffer->root_list, linked_compiler->config->listing_file, 0);
    }
    colorized_buffer->write_block_to_object_file(block->block_name->variable_name, target);

    delete colorized_buffer;
    delete procedure_command_buffer;
    current_descriptors = global_descriptors;
}

void BackendContext::compile_plus(TreeNodeOperator* expression) {
    if (expression->left)
        compile_expression(expression->left);
    else
        state->register_stack.push_imm64(0);
    compile_expression(expression->right);
    state->register_stack.add();
}

void BackendContext::compile_minus(TreeNodeOperator* expression) {
    if (expression->left)
        compile_expression(expression->left);
    else
        state->register_stack.push_imm64(0);
    compile_expression(expression->right);
    state->register_stack.sub();
}

void BackendContext::compile_multiply(TreeNodeOperator* expression) {
    compile_expression(expression->left);
    compile_expression(expression->right);
    state->register_stack.mul();
}

void BackendContext::compile_divide(TreeNodeOperator* expression) {
    compile_expression(expression->left);
    compile_expression(expression->right);
    state->register_stack.div();
}

void BackendContext::compile_line(TreeNode* node) {
    if (node->type == TREE_NODE_TYPE_VAR_DEFINITION) {
        auto* var_definition = (TreeNodeVariableDefinition*)node;
        if (var_definition->variable_value)
            compile_assignment(var_definition->variable_name, var_definition->variable_value);
        else
            return;
    } else if (node->type == TREE_NODE_TYPE_CYCLE) {
        compile_cycle((TreeNodeCycle*)node);
    } else if (node->type == TREE_NODE_TYPE_CHECK) {
        compile_check((TreeNodeCheck*)node);
    } else {
        compile_expression(node);
    }
}

void BackendContext::compile_expression(TreeNode* node) {
    if (node->type == TREE_NODE_TYPE_OPERATOR) {
        auto* oper = (TreeNodeOperator*)node;

        switch (oper->oper_type) {
        case BONK_OPERATOR_ASSIGNMENT:
            compile_assignment((TreeNodeIdentifier*)oper->left, oper->right);
            break;
        case BONK_OPERATOR_PLUS:
            compile_plus(oper);
            break;
        case BONK_OPERATOR_MINUS:
            compile_minus(oper);
            break;
        case BONK_OPERATOR_MULTIPLY:
            compile_multiply(oper);
            break;
        case BONK_OPERATOR_DIVIDE:
            compile_divide(oper);
            break;
        case BONK_OPERATOR_OR:
            compile_or(oper);
            break;
        case BONK_OPERATOR_AND:
            compile_and(oper);
            break;
        case BONK_OPERATOR_EQUALS:
            compile_equals(oper);
            break;
        case BONK_OPERATOR_LESS_THAN:
            compile_less_than(oper);
            break;
        case BONK_OPERATOR_LESS_OR_EQUAL_THAN:
            compile_less_or_equal_than(oper);
            break;
        case BONK_OPERATOR_GREATER_THAN:
            compile_greater_than(oper);
            break;
        case BONK_OPERATOR_GREATER_OR_EQUAL_THAN:
            compile_greater_or_equal_than(oper);
            break;
        case BONK_OPERATOR_NOT_EQUAL:
            compile_not_equal(oper);
            break;
        case BONK_OPERATOR_BREK:
            compile_brek_statement(oper);
            break;
        case BONK_OPERATOR_REBONK:
            compile_rebonk_statement(oper);
            break;
        case BONK_OPERATOR_BONK:
            compile_bonk_statement(oper);
            break;
            //            case BONK_OPERATOR_PRINT:      compile_print           (oper); break; case
            //            BONK_OPERATOR_BAMS:       compile_inline_assembly (oper); break; default:
            //            compile_math_comparsion (oper);
        default:
            assert(false);
            break;
        }
    } else if (node->type == TREE_NODE_TYPE_CALL) {
        compile_call((TreeNodeCall*)node);
    } else if (node->type == TREE_NODE_TYPE_NUMBER) {
        state->register_stack.push_imm64(((TreeNodeNumber*)node)->integer_value);
    } else if (node->type == TREE_NODE_TYPE_IDENTIFIER) {
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
        linked_compiler->error_positioned(identifier->source_position,
                                          "'%s' is a block name, not variable",
                                          identifier->variable_name.c_str());
        return -1;
    }

    return variable->storage;
}

void BackendContext::push_state() const {
    auto new_command_list = procedure_command_buffer->next_command_list();
    state_stack.push_back({new_command_list});
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
    state_stack.push_back({procedure_command_buffer->root_list});
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
    compile_block(cycle->body);
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
    ParserPosition* position = identifier->source_position;
    linked_compiler->error_positioned(position, "variable '%s' is already defined",
                                      identifier->variable_name.c_str());
}

void BackendContext::error_undefined_reference(TreeNodeIdentifier* node) const {
    ParserPosition* position = node->source_position;
    linked_compiler->error_positioned(position, "undefined reference: '%s'",
                                      node->variable_name.c_str());
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
        compile_expression(oper->right);
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
    return can_use_fast_logic_on_operand(oper->left, oper->oper_type) &&
           can_use_fast_logic_on_operand(oper->right, oper->oper_type);
}

bool BackendContext::can_use_fast_logic_on_operand(TreeNode* operand, OperatorType oper_type) {
    if (operand->type == TREE_NODE_TYPE_OPERATOR) {
        if (((TreeNodeOperator*)operand)->oper_type == oper_type) {
            return can_use_fast_logic((TreeNodeOperator*)operand);
        }
    }
    return operand->type == TREE_NODE_TYPE_NUMBER || operand->type == TREE_NODE_TYPE_IDENTIFIER;
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
    if (oper->left->type == TREE_NODE_TYPE_OPERATOR &&
        ((TreeNodeOperator*)oper->left)->oper_type == oper_type) {
        // left-hand operand must be expression
        bool is_expr = compile_logic_recursive((TreeNodeOperator*)(oper->left), control_label,
                                               control_label_list, oper_type, check_for);
        assert(is_expr);
    } else {
        // left-hand operand must be expression
        bool is_expr = compile_logic_operand_recursive(oper->left, control_label,
                                                       control_label_list, oper_type, check_for);
        assert(is_expr);
    }

    if (oper->right->type == TREE_NODE_TYPE_OPERATOR &&
        ((TreeNodeOperator*)oper->right)->oper_type == oper_type) {
        return compile_logic_recursive((TreeNodeOperator*)(oper->right), control_label,
                                       control_label_list, oper_type, check_for);
    } else {
        return compile_logic_operand_recursive(oper->right, control_label, control_label_list,
                                               oper_type, check_for);
    }
}

void BackendContext::compile_jump_logical(TreeNodeOperator* oper) {
    bool is_and = oper->oper_type == BONK_OPERATOR_AND;
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
        compile_expression(oper->left);
        compile_expression(oper->right);
        state->register_stack.logical_or();
    } else {
        compile_jump_logical(oper);
    }
}

void BackendContext::compile_and(TreeNodeOperator* oper) {
    if (can_use_fast_logic(oper)) {
        compile_expression(oper->left);
        compile_expression(oper->right);
        state->register_stack.logical_and();
    } else {
        compile_jump_logical(oper);
    }
}

void BackendContext::compile_equals(TreeNodeOperator* oper) {
    compile_expression(oper->left);
    compile_expression(oper->right);
    state->register_stack.equals();
}

void BackendContext::compile_less_than(TreeNodeOperator* oper) {
    compile_expression(oper->left);
    compile_expression(oper->right);
    state->register_stack.less_than();
}

void BackendContext::compile_less_or_equal_than(TreeNodeOperator* oper) {
    compile_expression(oper->left);
    compile_expression(oper->right);
    state->register_stack.less_or_equal_than();
}

void BackendContext::compile_greater_than(TreeNodeOperator* oper) {
    compile_expression(oper->left);
    compile_expression(oper->right);
    state->register_stack.greater_than();
}

void BackendContext::compile_greater_or_equal_than(TreeNodeOperator* oper) {
    compile_expression(oper->left);
    compile_expression(oper->right);
    state->register_stack.greater_or_equal_than();
}

void BackendContext::compile_not_equal(TreeNodeOperator* oper) {
    compile_expression(oper->left);
    compile_expression(oper->right);
    state->register_stack.not_equal();
}

JumpCommand* BackendContext::append_oper_jmp_command(TreeNodeOperator* oper, bool inversed) {
    compile_expression(oper->left);
    compile_expression(oper->right);
    state->register_stack.compare();

    auto command = COMMAND_JMP;
    switch (oper->oper_type) {
    case BONK_OPERATOR_EQUALS:
        command = COMMAND_JE;
        break;
    case BONK_OPERATOR_LESS_THAN:
        command = COMMAND_JL;
        break;
    case BONK_OPERATOR_LESS_OR_EQUAL_THAN:
        command = COMMAND_JNG;
        break;
    case BONK_OPERATOR_GREATER_THAN:
        command = COMMAND_JG;
        break;
    case BONK_OPERATOR_GREATER_OR_EQUAL_THAN:
        command = COMMAND_JNL;
        break;
    case BONK_OPERATOR_NOT_EQUAL:
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

    if (node->type == TREE_NODE_TYPE_OPERATOR) {
        auto oper = (TreeNodeOperator*)node;
        switch (oper->oper_type) {
        case BONK_OPERATOR_EQUALS:
        case BONK_OPERATOR_LESS_THAN:
        case BONK_OPERATOR_LESS_OR_EQUAL_THAN:
        case BONK_OPERATOR_GREATER_THAN:
        case BONK_OPERATOR_GREATER_OR_EQUAL_THAN:
        case BONK_OPERATOR_NOT_EQUAL:
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

    auto jmp = compile_boolean_jump(node->condition, false);

    push_state();
    compile_block(node->check_body);
    pop_state();

    jmp->set_label(insert_label());
}

void BackendContext::compile_brek_statement(TreeNodeOperator* oper) {
    if (!cycle_container) {
        linked_compiler->error_positioned(oper->source_position, "brek operator outside of cycle");
        return;
    }

    pop_to_scope(cycle_container);
    state->current_command_list->commands.push_back(new JumpCommand(cycle_tail, COMMAND_JMP));
}

void BackendContext::compile_rebonk_statement(TreeNodeOperator* oper) {
    if (!cycle_body) {
        linked_compiler->error_positioned(oper->source_position,
                                          "rebonk operator outside of cycle");
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

TreeNode*
BackendContext::call_argument_list_get_value(
    TreeNodeList<TreeNodeCallParameter*>* argument_list, TreeNodeIdentifier* identifier) {
    if (!argument_list)
        return nullptr;

    std::list<TreeNodeCallParameter*>* list = &argument_list->list;

    for (auto i = list->begin(); i != list->end(); ++i) {
        auto* parameter = *i;

        if (parameter->parameter_name->contents_equal(identifier)) {
            return parameter->parameter_value;
        }
    }

    return nullptr;
}

TreeNode* BackendContext::compile_nth_argument(VariableFunction* func, TreeNodeList<TreeNodeCallParameter*>* argument_list, int i) {
    Variable* argument = func->argument_list->variables[i];

    TreeNode* value = call_argument_list_get_value(argument_list, argument->identifier);
    if (value == nullptr) {
        ParserPosition* position = func->identifier->source_position;
        linked_compiler->error_positioned(position, "'%s' requires contextual variable '%s'",
                                          func->identifier->variable_name.c_str(),
                                          argument->identifier->variable_name.c_str());
        return nullptr;
    }

    int stack_size = state->register_stack.stack.size();
    compile_expression(value);
    if (state->register_stack.stack.size() == stack_size) {
        linked_compiler->error_positioned(value->source_position,
                                          "expression does not return anything");
    }

    return value;
}

void BackendContext::compile_call(TreeNodeCall* call) {
    auto* argument_list = call->call_parameters;
    auto* function_name = (TreeNodeIdentifier*)call->call_function;

    Variable* var = scope_stack->get_variable(function_name, nullptr);

    if (var == nullptr) {
        error_undefined_reference(function_name);
        return;
    }

    if (var->type != VARIABLE_TYPE_FUNCTION) {
        ParserPosition* position = function_name->source_position;
        linked_compiler->error_positioned(position, "'%s' is not a function",
                                          function_name->variable_name.c_str());
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

    state->current_command_list->commands.push_back(AlignStackCommand::create_before(stack_arguments));

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

    state->current_command_list->commands.push_back(new RegPreserveCommand(caller_preserved_registers));

    // Now it's safe to call the function

    state->current_command_list->commands.push_back(new CallCommand(CommandParameterSymbol(true,
                                 target->get_symbol_from_name(call->call_function->variable_name)),
        arguments, register_arguments));

    state->current_command_list->commands.push_back(AlignStackCommand::create_after(stack_arguments));
    state->register_stack.push_reg64(
        procedure_command_buffer->descriptors.next_constrained_register(
            rax, state->current_command_list));
}

void BackendContext::write_block_definition(TreeNodeBlockDefinition* definition) const {
    if (definition->is_promise) {
        target->declare_external_symbol(definition->block_name->variable_name);
    } else {
        target->declare_internal_symbol(definition->block_name->variable_name);
    }
}

void BackendContext::write_global_var_definition(TreeNodeVariableDefinition* definition) const {
    target->declare_internal_symbol(definition->variable_name->variable_name);
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