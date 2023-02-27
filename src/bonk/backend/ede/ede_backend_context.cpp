
#include <list>
#include "ede_backend_context.hpp"

namespace bonk::ede_backend {

BackendContext::BackendContext(Compiler* compiler, FILE* destination) {
    scope_stack_offset = 0;
    processor_stack_usage = 0;
    linked_compiler = compiler;

    labels = 1;
    cycle_label_id = 0;
    target = destination;
}

void BackendContext::error_undefined_reference(TreeNodeIdentifier* node) {
    ParserPosition* position = node->source_position;
    linked_compiler->error_positioned(position, "undefined reference: '%s'",
                                      node->variable_name.c_str());
}

void BackendContext::reference_variable(TreeNodeIdentifier* identifier) {
    FieldList* scope = nullptr;
    Variable* variable = scope_stack.get_variable(identifier, &scope);

    if (variable == nullptr) {
        error_undefined_reference(identifier);
        return;
    }

    if (variable->type == VARIABLE_TYPE_FUNCTION) {
        linked_compiler->error_positioned(identifier->source_position,
                                          "'%s' is a block name, not variable",
                                          identifier->variable_name.c_str());
        return;
    }

    if (scope == scope_stack.scopes[0]) {
        fprintf(target, "[0x%lx]", variable->byte_offset);
    } else {
        if (variable->byte_offset == scope_stack_offset) {
            fprintf(target, "[rdx]");
        } else if (variable->byte_offset > scope_stack_offset) {
            fprintf(target, "[rdx + 0x%lx]", variable->byte_offset - scope_stack_offset);
        } else {
            fprintf(target, "[rdx - 0x%lx]", scope_stack_offset - variable->byte_offset);
        }
    }

    fprintf(target, " ; referencing '");
    variable->identifier->print(target);
    fprintf(target, "' field");
}

void BackendContext::compile_factor(TreeNode* node) {
    if (node->type == TREE_NODE_TYPE_NUMBER) {
        fprintf(target, "push.d %.20LG\n", ((TreeNodeNumber*)node)->float_value);
        processor_stack_usage += 8;
    } else if (node->type == TREE_NODE_TYPE_IDENTIFIER) {
        fprintf(target, "push.d ");
        reference_variable((TreeNodeIdentifier*)node);
        fprintf(target, "\n");
        processor_stack_usage += 8;
    } else {
        compile_expression(node, 8);
    }
}

void BackendContext::compile_term(TreeNode* node) {
    if (node->type == TREE_NODE_TYPE_OPERATOR) {
        TreeNodeOperator* oper = (TreeNodeOperator*)node;
        OperatorType operator_type = oper->oper_type;

        if (operator_type == BONK_OPERATOR_MULTIPLY || operator_type == BONK_OPERATOR_DIVIDE) {
            compile_term(oper->left);
            compile_term(oper->right);

            if (operator_type == BONK_OPERATOR_MULTIPLY) {
                fprintf(target, "mul.d\n");
                processor_stack_usage -= 8;
            } else {
                fprintf(target, "div.d\n");
                processor_stack_usage -= 8;
            }
            return;
        }
    }

    compile_factor(node);
}

void BackendContext::compile_math_expression(TreeNode* node) {
    if (node->type == TREE_NODE_TYPE_OPERATOR) {
        auto* oper = (TreeNodeOperator*)node;
        OperatorType operator_type = oper->oper_type;

        if (operator_type == BONK_OPERATOR_PLUS || operator_type == BONK_OPERATOR_MINUS) {
            if (oper->left) {
                compile_math_expression(oper->left);
            } else {
                fprintf(target, "push.d 0\n");
                processor_stack_usage += 8;
            }
            compile_math_expression(oper->right);

            if (operator_type == BONK_OPERATOR_PLUS) {
                fprintf(target, "add.d\n");
                processor_stack_usage -= 8;
            } else {
                fprintf(target, "sub.d\n");
                processor_stack_usage -= 8;
            }
            return;
        }
    }

    compile_term(node);
}

const char* BackendContext::get_comparsion_instruction(OperatorType operator_type) {
    switch (operator_type) {
    case BONK_OPERATOR_EQUALS:
        return "je";
    case BONK_OPERATOR_LESS_OR_EQUAL_THAN:
        return "jbe";
    case BONK_OPERATOR_GREATER_OR_EQUAL_THAN:
        return "jae";
    case BONK_OPERATOR_LESS_THAN:
        return "jb";
    case BONK_OPERATOR_GREATER_THAN:
        return "ja";
    case BONK_OPERATOR_NOT_EQUAL:
        return "jne";
    default:
        return nullptr;
    }
}

const char* BackendContext::get_comparsion_instruction_negated(OperatorType operator_type) {
    switch (operator_type) {
    case BONK_OPERATOR_EQUALS:
        return "jne";
    case BONK_OPERATOR_LESS_OR_EQUAL_THAN:
        return "ja";
    case BONK_OPERATOR_GREATER_OR_EQUAL_THAN:
        return "jb";
    case BONK_OPERATOR_LESS_THAN:
        return "jae";
    case BONK_OPERATOR_GREATER_THAN:
        return "jbe";
    case BONK_OPERATOR_NOT_EQUAL:
        return "je";
    default:
        return nullptr;
    }
}

void BackendContext::compile_math_comparsion(TreeNode* node) {
    if (node->type == TREE_NODE_TYPE_OPERATOR) {
        TreeNodeOperator* oper = (TreeNodeOperator*)node;
        OperatorType operator_type = oper->oper_type;

        const char* comparing_instruction = get_comparsion_instruction(operator_type);

        if (comparing_instruction != nullptr) {
            compile_math_comparsion(oper->left);
            compile_math_comparsion(oper->right);

            unsigned long label_id = next_label_id();

            fprintf(target,
                    "%s.d _cmp_%lu_ok\n"
                    "push.d 0\n"
                    "jmp _cmp_%lu_end\n"
                    "_cmp_%lu_ok:\n"
                    "push.d 1\n"
                    "_cmp_%lu_end:\n",
                    comparing_instruction, label_id, label_id, label_id, label_id);
            processor_stack_usage -= 8;
            return;
        }
    }

    compile_math_expression(node);
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

void BackendContext::compile_call(TreeNodeCall* call) {
    auto* argument_list = call->call_parameters;
    auto* function_name = (TreeNodeIdentifier*)call->call_function;

    Variable* var = scope_stack.get_variable(function_name, nullptr);

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
    FieldList* top_scope = scope_stack.top();

    unsigned long safe_zone_offset = function_arguments->variables.size() * 8;

    fprintf(target, " ; calling block %s\n", function_name->variable_name.c_str());
    fprintf(target, " ;  top frame size = %lu\n", top_scope->frame_size);
    fprintf(target, " ;  top frame offset = %lu\n", top_scope->byte_offset);
    fprintf(target, " ;  safe zone = %lu\n", safe_zone_offset);

    unsigned long argument_calculation_offset =
        top_scope->frame_size + top_scope->byte_offset + safe_zone_offset;

    if (argument_calculation_offset > 0) {
        fprintf(target,
                "push.ul (rdx + 0x%lx)\n"
                "pop.ul rdx\n",
                top_scope->frame_size + top_scope->byte_offset + safe_zone_offset);

        scope_stack_offset += top_scope->frame_size + top_scope->byte_offset + safe_zone_offset;
    }

    for (int i = 0; i < function_arguments->variables.size(); i++) {
        Variable* argument = function_arguments->variables[i];

        TreeNode* value = call_argument_list_get_value(argument_list, argument->identifier);
        if (value == nullptr) {
            ParserPosition* position = function_name->source_position;
            linked_compiler->error_positioned(position, "'%s' requires contextual variable '%s'",
                                              function_name->variable_name.c_str(),
                                              argument->identifier->variable_name.c_str());

            return;
        } else {
            compile_expression(value, 8);
            fprintf(target, "pop.d [rdx - 0x%lx] ; writing to argument '",
                    safe_zone_offset - i * 8);
            argument->identifier->print(target);
            fprintf(target, "'\n");
            processor_stack_usage -= 8;
        }
    }

    scope_stack_offset -= top_scope->frame_size + top_scope->byte_offset + safe_zone_offset;

    if (safe_zone_offset > 0) {
        fprintf(target,
                "push.ul (rdx - 0x%lx)\n"
                "pop.ul rdx\n",
                safe_zone_offset);
    }

    fprintf(target, "call _bs_%s\n", function_name->variable_name.c_str());

    if (top_scope->frame_size + top_scope->byte_offset > 0) {
        fprintf(target,
                "push.ul (rdx - 0x%lx)\n"
                "pop.ul rdx\n",
                top_scope->frame_size + top_scope->byte_offset);
    }

    processor_stack_usage += 8;

    free((void*)function_name->variable_name.c_str());
}

void BackendContext::compile_inline_assembly(TreeNodeOperator* node) {
    fprintf(target, "; inline assembly at line %lu:\n", node->source_position->line);

    TreeNodeIdentifier* inline_asm = (TreeNodeIdentifier*)node->left;

    TreeNodeIdentifier inline_variable{""};
    inline_variable.source_position = node->source_position;

    const char* asm_begin = inline_asm->variable_name.data();
    unsigned long asm_length = inline_asm->variable_name.size();

    bool was_whitespace = false;
    bool was_newline = true;

    while (asm_length) {
        char next = *(asm_begin++);
        asm_length--;

        if (next == ' ' || next == '\t') {
            if (was_whitespace || was_newline)
                continue;
            was_whitespace = true;
            was_newline = false;
            fputc(next, target);
            continue;
        }
        was_whitespace = false;

        if (next == '\n') {
            if (was_newline)
                continue;
            was_newline = true;
            fputc(next, target);
            continue;
        }
        was_newline = false;

        if (next == '%') {
            const char* variable_name_start = asm_begin;
            int variable_name_length = -1;

            do {
                variable_name_length++;
                next = *(asm_begin++);
                asm_length--;
            } while (next != '%');

            if (variable_name_length == 0) {
                linked_compiler->error_positioned(node->source_position,
                                                  "empty variable reference in inline assembly");
            } else {
                inline_variable.variable_name =
                    std::string(variable_name_start, variable_name_length);
                reference_variable(&inline_variable);
            }
            continue;
        }

        fputc(next, target);
    }

    if (!was_newline)
        fputc('\n', target);

    fprintf(target, "; inline assembly end:\n");
}

void BackendContext::compile_bonk_statement(TreeNodeOperator* node,
                                             unsigned long stack_bytes_expected) {
    if (stack_bytes_expected > 0) {
        linked_compiler->error_positioned(node->source_position, "bonk does not return anything");
    }

    compile_expression(node->right, 8);
    fprintf(target, "pop.d rbx\n"
                    "pop.ul rax\n"
                    "push.d rbx\n"
                    "push.ul rax\n"
                    "ret\n");
    processor_stack_usage -= 8;
}

void BackendContext::compile_brek_statement(TreeNodeOperator* node,
                                             unsigned long stack_bytes_expected) {
    if (cycle_label_id == 0) {
        linked_compiler->error_positioned(node->source_position, "brek statement outside cycle");
    } else {
        if (stack_bytes_expected > 0) {
            linked_compiler->error_positioned(node->source_position,
                                              "brek does not return anything");
        }
        fprintf(target, "jmp _cycle_%lu_end\n", cycle_label_id);
    }
}

void BackendContext::compile_rebonk_statement(TreeNodeOperator* node,
                                               unsigned long stack_bytes_expected) {
    if (cycle_label_id == 0) {
        linked_compiler->error_positioned(node->source_position, "rebonk statement outside cycle");
    } else {
        if (stack_bytes_expected > 0) {
            linked_compiler->error_positioned(node->source_position,
                                              "rebonk does not return anything");
        }
        fprintf(target, "jmp _cycle_%lu\n", cycle_label_id);
    }
}

void BackendContext::compile_assignment(TreeNodeIdentifier* variable, TreeNode* value,
                                         unsigned long stack_bytes_expected) {
    compile_expression(value, 8);
    if (stack_bytes_expected != 0) {
        fprintf(target, "pop.d rax\n"
                        "push.d rax\n"
                        "push.d rax\n");
        processor_stack_usage += 8;
    }
    fprintf(target, "pop.d ");
    reference_variable(variable);
    fprintf(target, "\n");
    processor_stack_usage -= 8;
}

void BackendContext::compile_cycle(TreeNodeCycle* node, unsigned long stack_bytes_expected) {
    if (stack_bytes_expected > 0) {
        linked_compiler->error_positioned(node->source_position, "cycle does not return anything");
    }

    unsigned long wrapping_cycle_id = cycle_label_id;
    unsigned long label_id = next_label_id();
    cycle_label_id = label_id;

    fprintf(target, "_cycle_%lu:\n", label_id);
    compile_block(node->body, false);
    fprintf(target,
            "jmp _cycle_%lu\n"
            "_cycle_%lu_end:\n",
            label_id, label_id);

    cycle_label_id = wrapping_cycle_id;
}

void BackendContext::compile_check(TreeNodeCheck* node, unsigned long stack_bytes_expected) {
    if (stack_bytes_expected > 0) {
        linked_compiler->error_positioned(node->source_position, "check does not return anything");
    }

    unsigned long label_id = next_label_id();

    const char* comparsion_instruction = nullptr;

    TreeNode* comparsion = node->condition;

    if (comparsion->type == TREE_NODE_TYPE_OPERATOR) {
        comparsion_instruction =
            get_comparsion_instruction_negated(((TreeNodeOperator*)comparsion)->oper_type);
    }

    if (comparsion_instruction == nullptr) {
        compile_expression(comparsion, 8);
        processor_stack_usage -= 8;
        comparsion_instruction = "jz";
    } else {
        compile_expression(((TreeNodeOperator*)comparsion)->left, 8);
        compile_expression(((TreeNodeOperator*)comparsion)->right, 8);
        processor_stack_usage -= 16;
    }

    if (node->or_body) {
        fprintf(target, "%s.d _if_%lu_else\n", comparsion_instruction, label_id);
        compile_block(node->check_body, false);
        fprintf(target,
                "jmp _if_%lu_end\n"
                "_if_%lu_else:\n",
                label_id, label_id);
        compile_block(node->or_body, false);
        fprintf(target, "_if_%lu_end:\n", label_id);
    } else {
        fprintf(target, "%s.d _if_%lu_end\n", comparsion_instruction, label_id);
        compile_block(node->check_body, false);
        fprintf(target, "_if_%lu_end:\n", label_id);
    }
}

void BackendContext::compile_logic(TreeNodeOperator* oper, unsigned long stack_bytes_expected) {
    unsigned long label_id = next_label_id();

    compile_expression(oper->left, 8);

    if (stack_bytes_expected > 0) {
        fprintf(target, "pop.d rax\n"
                        "push.d rax\n"
                        "push.d rax\n");
    }

    if (oper->oper_type == BONK_OPERATOR_OR) {
        fprintf(target, "jnz.d _log_%lu_ok\n", label_id);
    } else {
        fprintf(target, "jz.d _log_%lu_ok\n", label_id);
    }

    if (stack_bytes_expected > 0) {
        fprintf(target, "push.ul (rsp - 0x8)\n"
                        "pop.ul rsp\n");
    }

    processor_stack_usage -= 8;
    compile_expression(oper->right, stack_bytes_expected);
    fprintf(target, "_log_%lu_ok:\n", label_id);
}

void BackendContext::compile_print(TreeNodeOperator* node) {
    compile_expression(node->right, 8);
    fprintf(target, "out.d\n");
}

unsigned long BackendContext::next_label_id() {
    return labels++;
}

void BackendContext::compile_expression(TreeNode* node, unsigned long stack_bytes_expected) {
    unsigned long saved_stack_usage = processor_stack_usage + stack_bytes_expected;

    if (node->type == TREE_NODE_TYPE_OPERATOR) {
        auto* oper = (TreeNodeOperator*)node;

        switch (oper->oper_type) {
        case BONK_OPERATOR_PRINT:
            compile_print(oper);
            break;
        case BONK_OPERATOR_BONK:
            compile_bonk_statement(oper, stack_bytes_expected);
            break;
        case BONK_OPERATOR_BREK:
            compile_brek_statement(oper, stack_bytes_expected);
            break;
        case BONK_OPERATOR_REBONK:
            compile_rebonk_statement(oper, stack_bytes_expected);
            break;
        case BONK_OPERATOR_ASSIGNMENT:
            compile_assignment((TreeNodeIdentifier*)oper->left, oper->right,
                               stack_bytes_expected);
            break;
        case BONK_OPERATOR_OR:
        case BONK_OPERATOR_AND:
            compile_logic(oper, stack_bytes_expected);
            break;
        case BONK_OPERATOR_BAMS:
            compile_inline_assembly(oper);
            break;
        default:
            compile_math_comparsion(oper);
            break;
        }
    } else if (node->type == TREE_NODE_TYPE_VAR_DEFINITION) {
        auto* var_definition = (TreeNodeVariableDefinition*)node;
        if (var_definition->variable_value)
            compile_assignment(var_definition->variable_name, var_definition->variable_value,
                               stack_bytes_expected);
        else
            return;
    } else if (node->type == TREE_NODE_TYPE_CALL) {
        compile_call((TreeNodeCall*)node);
    } else if (node->type == TREE_NODE_TYPE_CYCLE) {
        compile_cycle((TreeNodeCycle*)node, stack_bytes_expected);
    } else if (node->type == TREE_NODE_TYPE_CHECK) {
        compile_check((TreeNodeCheck*)node, stack_bytes_expected);
    } else {
        compile_math_expression(node);
    }

    unsigned long stack_bytes_to_pop = processor_stack_usage - saved_stack_usage;

    if (stack_bytes_to_pop > 0) {
        fprintf(target, "push.ul (rsp - %lu)\npop.ul rsp\n", stack_bytes_to_pop);
        processor_stack_usage = saved_stack_usage;
    }
}

void BackendContext::error_already_defined(TreeNodeIdentifier* identifier) {
    ParserPosition* position = identifier->source_position;
    linked_compiler->error_positioned(position, "variable '%s' is already defined",
                                      identifier->variable_name.c_str());
}

void BackendContext::field_list_declare_variable(TreeNodeVariableDefinition* node) {

    Variable* var = scope_stack.get_variable(node->variable_name, nullptr);

    if (var == nullptr) {
        var = new VariableNumber(node->variable_name);

        FieldList* top_scope = scope_stack.top();

        if (!var)
            linked_compiler->out_of_memory();
        else
            top_scope->add_variable(var);
    } else {
        error_already_defined(node->variable_name);
    }
}

FieldList* BackendContext::field_list_find_block_parameters(TreeNodeBlockDefinition* block) {
    FieldList* argument_list = new FieldList();
    if (!argument_list)
        return nullptr;

    auto* list = &block->body->list;

    for (auto i = list->begin(); i != list->end(); ++i) {
        auto* next_node = *i;

        TreeNodeType node_type = next_node->type;
        if (node_type != TREE_NODE_TYPE_VAR_DEFINITION)
            break;

        auto* var_definition = (TreeNodeVariableDefinition*)next_node;

        if (!var_definition->is_contextual)
            break;

        Variable* number = new VariableNumber(var_definition->variable_name);

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

    Variable* var = scope_stack.get_variable(identifier, nullptr);

    if (var == nullptr) {
        FieldList* argument_list = field_list_find_block_parameters(node);
        VariableFunction* func = nullptr;

        if (argument_list)
            func = new VariableFunction(identifier, argument_list);

        FieldList* top_scope = scope_stack.top();

        if (!func || !top_scope->add_variable((Variable*)func))
            linked_compiler->out_of_memory();
    } else {
        error_already_defined(identifier);
    }
}

FieldList* BackendContext::read_scope_variables(TreeNodeList<TreeNode*>* node,
                                                  bool reset_frame_offset) {

    FieldList* scope = new FieldList();
    if (!scope || !scope_stack.push_scope(scope)) {
        linked_compiler->out_of_memory();
        return nullptr;
    }

    if (reset_frame_offset)
        scope->byte_offset = 0;

    std::list<TreeNode*>* list = &node->list;

    for (auto i = list->begin(); i != list->end(); ++i) {
        auto* next_node = *i;

        TreeNodeType node_type = next_node->type;
        if (node_type == TREE_NODE_TYPE_VAR_DEFINITION) {
            field_list_declare_variable((TreeNodeVariableDefinition*)next_node);
        } else if (node_type == TREE_NODE_TYPE_BLOCK_DEFINITION) {
            field_list_declare_block((TreeNodeBlockDefinition*)next_node);
        }

        if (linked_compiler->state) {
            return nullptr;
        }
    }

    return scope;
}

void BackendContext::compile_block(TreeNodeList<TreeNode*>* node, bool reset_frame_offset) {
    FieldList* scope = read_scope_variables(node, reset_frame_offset);

    if (!linked_compiler->state) {
        std::list<TreeNode*>* list = &node->list;

        for (auto i = list->begin(); i != list->end(); ++i) {
            auto* next_node = *i;
            compile_expression(next_node, 0);
        }
    }

    if (scope) {
        scope_stack.pop_scope();
    }
}

void BackendContext::compile_callable_block(TreeNodeBlockDefinition* node) {
    fprintf(target, "; block ");
    node->block_name->print(target);
    fprintf(target, "\n");

    fprintf(target, "_bs_");
    node->block_name->print(target);
    fprintf(target, ":\n");

    compile_block(node->body, true);

    fprintf(target, "pop.ul rax\n"
                    "push.d 0\n"
                    "push.ul rax\n"
                    "ret\n");
}

void BackendContext::compile_program(TreeNodeList<TreeNode*>* node) {

    FieldList* scope = read_scope_variables(node, false);

    if (!linked_compiler->state) {

        std::list<TreeNode*>* list = &node->list;

        for (auto i = list->begin(); i != list->end(); ++i) {
            auto* next_node = *i;

            if (next_node->type == TREE_NODE_TYPE_VAR_DEFINITION) {
                auto* var_definition = (TreeNodeVariableDefinition*)next_node;
                if (var_definition->variable_value) {
                    compile_assignment(var_definition->variable_name,
                                       var_definition->variable_value, 0);
                }
            }
        }

        if (scope->frame_size > 0) {
            fprintf(target,
                    "push.ul (rdx + 0x%lx)\n"
                    "pop.ul rdx\n",
                    scope->frame_size);
        }

        fprintf(target, "call _bs_main\n");
        fprintf(target, "hlt\n");

        for (auto i = node->list.begin(); i != node->list.end(); ++i) {
            auto* next_node = *i;

            if (next_node->type == TREE_NODE_TYPE_BLOCK_DEFINITION) {
                compile_callable_block((TreeNodeBlockDefinition*)next_node);
            }
        }
    }

    if (scope) {
        scope_stack.pop_scope();
    }

    fprintf(target, "hlt\n");
}

} // namespace bonk::ede_backend