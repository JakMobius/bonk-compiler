
#include "hir_generator_visitor.hpp"
#include "hir.hpp"
#include "hir_base_block_separator.hpp"

void bonk::HIRGeneratorVisitor::visit(bonk::TreeNodeProgram* node) {
    ASTVisitor::visit(node);
}

void bonk::HIRGeneratorVisitor::visit(bonk::TreeNodeHelp* node) {
    ASTVisitor::visit(node);
}

void bonk::HIRGeneratorVisitor::visit(bonk::TreeNodeIdentifier* node) {
    int id = middle_end.id_table.get_id(node);
    register_stack.push_back(id);
}

void bonk::HIRGeneratorVisitor::visit(bonk::TreeNodeBlockDefinition* node) {
    auto old_block = current_block_definition;
    current_block_definition = node;

    current_program->create_procedure();
    current_procedure = &current_program->procedures.back();
    current_procedure->create_base_block();
    current_base_block = &current_procedure->base_blocks.back();

    int id = middle_end.id_table.get_id(node);
    auto type = (BlokType*)middle_end.type_table.get_type(node);
    HIRDataType return_type = convert_type_to_hir(type->return_type.get());

    auto procedure_header = current_base_block->instruction<HIRProcedure>(id, return_type);

    if (node->block_parameters) {
        for (auto& parameter : node->block_parameters->parameters) {
            if (parameter) {
                int parameter_id = middle_end.id_table.get_id(parameter.get());
                auto parameter_type = (BlokType*)middle_end.type_table.get_type(parameter.get());
                HIRDataType hir_parameter_type = convert_type_to_hir(parameter_type);
                procedure_header->parameters.push_back({hir_parameter_type, parameter_id});
            }
        }
    }

    current_base_block->instructions.push_back(procedure_header);

    node->body->accept(this);
    current_block_definition = old_block;
}

void bonk::HIRGeneratorVisitor::visit(bonk::TreeNodeVariableDefinition* node) {
    if (node->variable_value == nullptr)
        return;

    int variable_id = middle_end.id_table.get_id(node);
    auto variable_type = (BlokType*)middle_end.type_table.get_type(node);
    HIRDataType hir_variable_type = convert_type_to_hir(variable_type);

    // Count the right part of the assignment
    node->variable_value->accept(this);

    // Get the result of the right part of the assignment
    auto result = register_stack.back();
    register_stack.pop_back();

    // Move the result to the variable
    auto instruction = current_base_block->instruction<HIROperation>();
    instruction->operation_type = HIROperationType::assign;
    instruction->result_type = hir_variable_type;
    instruction->operand_type = hir_variable_type;
    instruction->target = variable_id;
    instruction->left = result;
    current_base_block->instructions.push_back(instruction);
}

void bonk::HIRGeneratorVisitor::visit(bonk::TreeNodeParameterList* node) {
    ASTVisitor::visit(node);
}

void bonk::HIRGeneratorVisitor::visit(bonk::TreeNodeParameterListItem* node) {
    ASTVisitor::visit(node);
}

void bonk::HIRGeneratorVisitor::visit(bonk::TreeNodeCodeBlock* node) {
    for (auto& element : node->body) {
        if (element) {
            element->accept(this);
            register_stack.clear();
        }
    }
}

void bonk::HIRGeneratorVisitor::visit(bonk::TreeNodeArrayConstant* node) {
    assert(!"Cannot compile this just yet");
}

void bonk::HIRGeneratorVisitor::visit(bonk::TreeNodeNumberConstant* node) {
    ASTVisitor::visit(node);

    int id = middle_end.id_table.get_unused_id();
    float value = node->double_value;
    auto instruction = current_base_block->instruction<HIRConstantLoad>(id, value);
    current_base_block->instructions.push_back(instruction);
    register_stack.push_back(id);
}

void bonk::HIRGeneratorVisitor::visit(bonk::TreeNodeStringConstant* node) {
    int id = middle_end.id_table.get_unused_id();

    // TODO: find the string in the string table and get its pointer
    uint64_t string_pointer = 0;
    auto instruction = current_base_block->instruction<HIRConstantLoad>(id, string_pointer);
    current_base_block->instructions.push_back(instruction);
    register_stack.push_back(id);
}

void bonk::HIRGeneratorVisitor::visit(bonk::TreeNodeBinaryOperation* node) {
    ASTVisitor::visit(node);

    Type* left_type = middle_end.type_table.get_type(node->left.get());
    Type* right_type = middle_end.type_table.get_type(node->right.get());

    int left = register_stack.back();
    register_stack.pop_back();
    int right = register_stack.back();
    register_stack.pop_back();

    // Assume that the operation is done on the numbers, just for now

    if (left_type->kind == TypeKind::primitive && right_type->kind == TypeKind::primitive) {

        auto left_primitive = (TrivialType*)left_type;
        auto right_primitive = (TrivialType*)right_type;

        if (left_primitive->primitive_type == right_primitive->primitive_type) {
            auto type = left_primitive->primitive_type;

            if (type == PrimitiveType::t_flot || type == PrimitiveType::t_nubr) {
                HIRDataType hir_type = convert_type_to_hir(left_type);
                HIROperationType operation_type = convert_operation_to_hir(node->operator_type);

                int id = middle_end.id_table.get_unused_id();
                auto instruction = current_base_block->instruction<HIROperation>();
                instruction->operation_type = operation_type;
                instruction->result_type = hir_type;
                instruction->operand_type = hir_type;
                instruction->target = id;
                instruction->left = left;
                instruction->right = right;
                current_base_block->instructions.push_back(instruction);
                register_stack.push_back(id);
                return;
            }
        }
    }

    assert(!"Cannot compile this just yet");
}

void bonk::HIRGeneratorVisitor::visit(bonk::TreeNodeUnaryOperation* node) {
    ASTVisitor::visit(node);

    Type* type = middle_end.type_table.get_type(node->operand.get());

    int operand = register_stack.back();

    if (type->kind == TypeKind::primitive) {
        auto primitive = (TreeNodePrimitiveType*)type;
        if (primitive->primitive_type == PrimitiveType::t_nubr ||
            primitive->primitive_type == PrimitiveType::t_flot) {
            HIRDataType hir_type = convert_type_to_hir(type);

            int zero_operand = middle_end.id_table.get_unused_id();
            HIRConstantLoad* zero_instruction = nullptr;

            if (primitive->primitive_type == PrimitiveType::t_nubr) {
                zero_instruction =
                    current_base_block->instruction<HIRConstantLoad>(zero_operand, (uint32_t)0);
            } else {
                zero_instruction =
                    current_base_block->instruction<HIRConstantLoad>(zero_operand, (float)0.0f);
            }

            current_base_block->instructions.push_back(zero_instruction);

            int id = middle_end.id_table.get_unused_id();
            auto instruction = current_base_block->instruction<HIROperation>();
            instruction->operation_type = HIROperationType::minus;
            instruction->result_type = hir_type;
            instruction->operand_type = hir_type;
            instruction->target = id;
            instruction->left = zero_operand;
            instruction->right = operand;

            current_base_block->instructions.push_back(instruction);
            register_stack.push_back(id);
            return;
        }
    }

    assert(!"Cannot compile this just yet");
}

void bonk::HIRGeneratorVisitor::visit(bonk::TreeNodeHiveAccess* node) {
    //    ASTVisitor::visit(node);

    assert("Hive access is not implemented yet");
}

void bonk::HIRGeneratorVisitor::visit(bonk::TreeNodeBonkStatement* node) {
    ASTVisitor::visit(node);

    auto instruction = current_base_block->instruction<HIRReturn>();

    if (node->expression) {
        Type* type = middle_end.type_table.get_type(node->expression.get());
        int id = register_stack.back();
        register_stack.pop_back();
        instruction->return_value = id;
        instruction->return_type = convert_type_to_hir(type);
    }

    current_base_block->instructions.push_back(instruction);
}

void bonk::HIRGeneratorVisitor::visit(bonk::TreeNodeLoopStatement* node) {
    auto old_loop_context = current_loop_context;

    // Setup loop variables
    node->loop_parameters->accept(this);

    int loop_start_id = middle_end.id_table.get_id(node);
    int loop_end_id = middle_end.id_table.get_unused_id();

    current_loop_context =
        HIRLoopContext{.loop_start_label = loop_start_id, .loop_end_label = loop_end_id};

    // Insert loop start label
    auto loop_start_label = current_base_block->instruction<HIRLabel>(loop_start_id);
    current_base_block->instructions.push_back(loop_start_label);

    // Compile loop body
    node->body->accept(this);

    // Insert loop end label
    auto loop_end_label = current_base_block->instruction<HIRLabel>(loop_end_id);
    current_base_block->instructions.push_back(loop_end_label);

    current_loop_context = old_loop_context;
}

void bonk::HIRGeneratorVisitor::visit(bonk::TreeNodeBrekStatement* node) {
    assert(current_loop_context.has_value());

    auto instruction = current_base_block->instruction<HIRJump>();
    instruction->label_id = current_loop_context->loop_end_label;
    current_base_block->instructions.push_back(instruction);
}

void bonk::HIRGeneratorVisitor::visit(bonk::TreeNodeHiveDefinition* node) {
    auto old_hive = current_hive_definition;
    current_hive_definition = node;
    ASTVisitor::visit(node);
    current_hive_definition = old_hive;
}

void bonk::HIRGeneratorVisitor::visit(bonk::TreeNodeCall* node) {

    auto callee = node->callee.get();

    if (callee->type == TreeNodeType::n_identifier) {
        auto identifier = (TreeNodeIdentifier*)callee;
        auto definition = middle_end.symbol_table.symbol_definitions[identifier];
        auto identifier_type = middle_end.type_table.get_type(definition);

        assert(identifier_type->kind == TypeKind::blok);

        auto block_type = (BlokType*)identifier_type;

        int label_id = middle_end.id_table.get_id(definition);

        for (auto it = block_type->parameters.rbegin(); it != block_type->parameters.rend(); it++) {
            auto& parameter = *it;
            auto& parameter_name = parameter->variable_name;

            TreeNode* parameter_value = nullptr;

            if (node->arguments) {
                for (auto& argument : node->arguments->parameters) {
                    if (argument->parameter_name->identifier_text ==
                        parameter_name->identifier_text) {
                        parameter_value = argument->parameter_value.get();
                        break;
                    }
                }
            }

            if (parameter_value) {
                parameter_value->accept(this);
            } else {
                assert(parameter->variable_value);
                parameter->variable_value->accept(this);
            }
        }

        for (auto& argument : block_type->parameters) {
            auto argument_type = middle_end.type_table.get_type(argument);
            auto argument_id = register_stack.back();
            register_stack.pop_back();

            auto instruction = current_base_block->instruction<HIRParameter>();
            instruction->parameter = argument_id;
            instruction->type = convert_type_to_hir(argument_type);
            current_base_block->instructions.push_back(instruction);
        }

        auto return_register = middle_end.id_table.get_unused_id();
        auto instruction = current_base_block->instruction<HIRCall>();
        instruction->procedure_label_id = label_id;
        instruction->return_value = return_register;
        instruction->return_type = convert_type_to_hir(block_type->return_type.get());
        current_base_block->instructions.push_back(instruction);

        register_stack.push_back(return_register);

    } else {
        assert(!"Cannot call function by pointer yet");
    }
}

std::unique_ptr<bonk::IRProgram> bonk::HIRGeneratorVisitor::generate(bonk::TreeNode* ast) {
    auto program = std::make_unique<IRProgram>(middle_end.id_table, middle_end.symbol_table);
    current_program = program.get();
    ast->accept(this);

    HIRBaseBlockSeparator::separate_blocks(*program);
    return program;
}

bonk::HIROperationType bonk::HIRGeneratorVisitor::convert_operation_to_hir(OperatorType type) {
    switch (type) {
    case OperatorType::o_plus:
        return HIROperationType::plus;
    case OperatorType::o_minus:
        return HIROperationType::minus;
    case OperatorType::o_multiply:
        return HIROperationType::multiply;
    case OperatorType::o_divide:
        return HIROperationType::divide;
    case OperatorType::o_assign:
        return HIROperationType::assign;
    case OperatorType::o_equal:
        return HIROperationType::equal;
    case OperatorType::o_not_equal:
        return HIROperationType::not_equal;
    case OperatorType::o_less:
        return HIROperationType::less;
    case OperatorType::o_less_equal:
        return HIROperationType::less_equal;
    case OperatorType::o_greater:
        return HIROperationType::greater;
    case OperatorType::o_greater_equal:
        return HIROperationType::greater_equal;
    case OperatorType::o_and:
        return HIROperationType::and_op;
    case OperatorType::o_or:
        return HIROperationType::or_op;
    default:
        assert(!"Unsupported operation");
    }
}

bonk::HIRDataType bonk::HIRGeneratorVisitor::convert_type_to_hir(bonk::Type* type) {
    switch (type->kind) {

    case TypeKind::primitive: {
        auto primitive = ((TrivialType*)type)->primitive_type;

        switch (primitive) {
        case PrimitiveType::t_strg:
            // Pointer type
            return HIRDataType::dword;
        case PrimitiveType::t_nubr:
            return HIRDataType::word;
        case PrimitiveType::t_flot:
            return HIRDataType::float32;
        default:
            assert(false);
        }
    }
    case TypeKind::hive:
    case TypeKind::many:
    case TypeKind::blok:
        // Pointer type
        return HIRDataType::dword;

    case TypeKind::nothing:
        // Base type
        return HIRDataType::word;

    default:
        assert(!"Unsupported type");
    }
}
