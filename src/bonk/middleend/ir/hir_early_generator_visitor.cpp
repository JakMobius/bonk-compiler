
#include "hir_early_generator_visitor.hpp"
#include "bonk/middleend/annotators/basic_symbol_annotator.hpp"
#include "bonk/middleend/annotators/type_visitor.hpp"
#include "hir.hpp"
#include "hir_base_block_separator.hpp"

void bonk::HIREarlyGeneratorVisitor::visit(bonk::TreeNodeProgram* node) {
    ASTVisitor::visit(node);
}

void bonk::HIREarlyGeneratorVisitor::visit(bonk::TreeNodeHelp* node) {
    ASTVisitor::visit(node);
}

void bonk::HIREarlyGeneratorVisitor::visit(bonk::TreeNodeIdentifier* node) {
    // If identifier holds reference to a hive, increase reference counter

    auto id = middle_end.id_table.get_id(node);

    auto type = middle_end.type_table.get_type(node);
    if (type->kind == TypeKind::hive) {
        auto instruction = current_base_block->instruction<HIRIncRefCounter>();
        instruction->address = id;
        current_base_block->instructions.push_back(instruction);
    }

    push_value(id);
}

void bonk::HIREarlyGeneratorVisitor::visit(bonk::TreeNodeBlockDefinition* node) {
    alive_scopes.emplace_back();
    alive_scopes.back().block = node;

    auto old_block = current_block_definition;
    current_block_definition = node;

    current_program->create_procedure();
    current_procedure = current_program->procedures.back().get();
    current_procedure->create_base_block();
    current_base_block = current_procedure->base_blocks.back().get();

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

    if (node->body) {
        node->body->accept(this);
        auto instruction = current_base_block->instruction<HIRReturn>();
        current_base_block->instructions.push_back(instruction);
    } else {
        procedure_header->is_external = true;
    }
    current_block_definition = old_block;

    alive_scopes.pop_back();
}

void bonk::HIREarlyGeneratorVisitor::visit(bonk::TreeNodeVariableDefinition* node) {
    if (!current_block_definition) {
        // It could be a global variable or a field of a hive
        return;
    }

    alive_scopes.back().alive_variables.push_back(node);

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
    instruction->left = result.register_id;
    current_base_block->instructions.push_back(instruction);
}

void bonk::HIREarlyGeneratorVisitor::visit(bonk::TreeNodeParameterList* node) {
    ASTVisitor::visit(node);
}

void bonk::HIREarlyGeneratorVisitor::visit(bonk::TreeNodeParameterListItem* node) {
    ASTVisitor::visit(node);
}

void bonk::HIREarlyGeneratorVisitor::visit(bonk::TreeNodeCodeBlock* node) {
    alive_scopes.emplace_back();
    alive_scopes.back().block = node;

    for (auto& element : node->body) {
        if (element) {
            element->accept(this);

            // If the node is an expression of hive type, its reference counter must be
            // decreased

            // This check is needed because the node could be a variable definition
            // or a bonk statement
            if (!register_stack.empty()) {
                auto statement_type = middle_end.type_table.get_type(element.get());
                if (statement_type->kind == TypeKind::hive) {

                    auto reference =
                        load_value(register_stack.back(), convert_type_to_hir(statement_type));

                    auto instruction = current_base_block->instruction<HIRDecRefCounter>();
                    instruction->address = reference;
                    instruction->hive_definition = ((HiveType*)statement_type)->hive_definition;
                    current_base_block->instructions.push_back(instruction);
                }
            }

            register_stack.clear();
        }
    }

    kill_alive_variables(node);
    alive_scopes.pop_back();
}

void bonk::HIREarlyGeneratorVisitor::visit(bonk::TreeNodeArrayConstant* node) {
    assert(!"Cannot compile this just yet");
}

void bonk::HIREarlyGeneratorVisitor::visit(bonk::TreeNodeNumberConstant* node) {
    // Get type of the constant
    Type* type = middle_end.type_table.get_type(node);

    HIRDataType hir_type = convert_type_to_hir(type);

    int id = middle_end.id_table.get_unused_id();
    auto instruction = current_base_block->instruction<HIRConstantLoad>(id, 0, hir_type);

    switch (hir_type) {
    case HIRDataType::unset:
        assert(false);

    case HIRDataType::byte:
        instruction->constant = (uint8_t)node->contents.integer_value;
        break;
    case HIRDataType::hword:
        instruction->constant = (uint16_t)node->contents.integer_value;
        break;
    case HIRDataType::word:
        instruction->constant = (uint32_t)node->contents.integer_value;
        break;
    case HIRDataType::dword:
        instruction->constant = (uint64_t)node->contents.integer_value;
        break;
    case HIRDataType::float32: {
        auto value = (float)node->contents.double_value;
        instruction->constant = *(uint32_t*)&value;
        break;
    }
    case HIRDataType::float64: {
        auto value = (double)node->contents.double_value;
        instruction->constant = *(uint64_t*)&value;
        break;
    }
    }

    current_base_block->instructions.push_back(instruction);
    push_value(id);
}

void bonk::HIREarlyGeneratorVisitor::visit(bonk::TreeNodeStringConstant* node) {
    int id = middle_end.id_table.get_unused_id();

    // TODO: find the string in the string table and get its pointer
    int64_t string_pointer = 0;
    auto instruction = current_base_block->instruction<HIRConstantLoad>(id, string_pointer);
    current_base_block->instructions.push_back(instruction);
    push_value(id);
}

void bonk::HIREarlyGeneratorVisitor::visit(bonk::TreeNodeBinaryOperation* node) {

    if (node->operator_type == OperatorType::o_and || node->operator_type == OperatorType::o_or) {
        compile_lazy_logic(node);
        return;
    }

    Type* left_type = middle_end.type_table.get_type(node->left.get());
    Type* right_type = middle_end.type_table.get_type(node->right.get());

    ASTVisitor::visit(node);

    auto right = register_stack.back();
    register_stack.pop_back();
    auto left = register_stack.back();
    register_stack.pop_back();

    assert(left_type->kind != TypeKind::unset && right_type->kind != TypeKind::unset);

    HIRDataType hir_type = convert_type_to_hir(left_type);
    HIRDataType result_type = convert_type_to_hir(middle_end.type_table.get_type(node));
    HIROperationType operation_type = convert_operation_to_hir(node->operator_type);

    if (operation_type == HIROperationType::assign) {

        bool is_hive_reference = left_type->kind == TypeKind::hive;
        assert(is_hive_reference == (right_type->kind == TypeKind::hive));

        auto right_loaded = load_value(right, hir_type);

        IRRegister left_loaded = 0;

        if (is_hive_reference) {
            // Increase the reference counter of the right-side hive, because
            // the assignment operator returns a temporary reference to the right-hand hive,
            // and the left-hand hive will now also have a reference to the right-hand hive
            // (reference 2)
            auto instruction = current_base_block->instruction<HIRIncRefCounter>();
            instruction->address = right_loaded;
            current_base_block->instructions.push_back(instruction);

            // Save the hive to decrease reference count later
            left_loaded = load_value(left, hir_type);
        }

        if (left.is_reference) {
            auto instruction = current_base_block->instruction<HIRMemoryStore>();
            instruction->address = left.register_id;
            instruction->value = right_loaded;
            instruction->type = hir_type;
            push_value(right_loaded);
            current_base_block->instructions.push_back(instruction);
        } else {
            auto instruction = current_base_block->instruction<HIROperation>();
            instruction->operation_type = operation_type;
            instruction->operand_type = hir_type;
            instruction->result_type = result_type;
            instruction->target = left.register_id;
            instruction->left = right_loaded;
            push_value(left.register_id);
            current_base_block->instructions.push_back(instruction);
        }

        if (is_hive_reference) {
            // Decrease the reference counter of the left-side hive twice, because
            //  1) The temporary reference to the right-hand hive is dead now
            //  2) The left-hand hive now has a reference to the right-hand hive

            for (int i = 0; i < 2; i++) {
                auto instruction = current_base_block->instruction<HIRDecRefCounter>();
                instruction->address = left_loaded;
                instruction->hive_definition = ((HiveType*)left_type)->hive_definition;
                current_base_block->instructions.push_back(instruction);
            }
        }

    } else {
        assert(left_type->kind == TypeKind::primitive);
        assert(right_type->kind == TypeKind::primitive);

        int id = middle_end.id_table.get_unused_id();

        auto instruction = current_base_block->instruction<HIROperation>();
        instruction->operation_type = operation_type;
        instruction->operand_type = hir_type;
        instruction->result_type = result_type;
        instruction->target = id;
        instruction->left = load_value(left, hir_type);
        instruction->right = load_value(right, hir_type);
        push_value(id);
        current_base_block->instructions.push_back(instruction);
    }
}

void bonk::HIREarlyGeneratorVisitor::compile_lazy_logic(TreeNodeBinaryOperation* node) {
    Type* left_type = middle_end.type_table.get_type(node->left.get());
    Type* right_type = middle_end.type_table.get_type(node->right.get());

    assert(left_type->kind == TypeKind::primitive);
    assert(right_type->kind == TypeKind::primitive || right_type->kind == TypeKind::nothing);

    // Compile left part, and only compile right part if it's needed
    node->left->accept(this);

    int left = load_value(register_stack.back(), convert_type_to_hir(left_type));
    register_stack.pop_back();

    int true_label = middle_end.id_table.get_unused_id();
    int false_label = middle_end.id_table.get_unused_id();
    int end_label = middle_end.id_table.get_unused_id();

    HIRJumpNZ* jump1 = nullptr;

    if (node->operator_type == OperatorType::o_and) {
        jump1 = current_base_block->instruction<HIRJumpNZ>(left, true_label, false_label);
    } else {
        jump1 = current_base_block->instruction<HIRJumpNZ>(left, false_label, true_label);
    }

    int result = middle_end.id_table.get_unused_id();

    current_base_block->instructions.push_back(jump1);
    current_base_block->instructions.push_back(
        current_base_block->instruction<HIRLabel>(true_label));

    node->right->accept(this);

    if (right_type->kind == TypeKind::nothing) {
        // If right part is nothing, the or/and construction
        // should work like an 'if' statement, like this:
        // ((cond and true_branch) or false_branch)
        // So, if cond turned out to be false, the value of the
        // (cond and true_branch) expression should be false,
        // and vice versa. So the 'cond' value is pushed to the
        // register stack.

        // Move left to result
        auto instruction = current_base_block->instruction<HIROperation>();
        instruction->operation_type = HIROperationType::assign;
        instruction->result_type = convert_type_to_hir(left_type);
        instruction->operand_type = instruction->operand_type;
        instruction->target = result;
        instruction->left = left;
        current_base_block->instructions.push_back(instruction);
    } else {
        // If it's an ordinary or/and construction, its value
        // should actually be calculated.
        int right = load_value(register_stack.back(), convert_type_to_hir(right_type));
        register_stack.pop_back();

        HIRDataType hir_type = convert_type_to_hir(left_type);
        HIROperationType operation_type = convert_operation_to_hir(node->operator_type);

        auto instruction = current_base_block->instruction<HIROperation>();
        instruction->operation_type = operation_type;
        instruction->result_type = hir_type;
        instruction->operand_type = hir_type;
        instruction->target = result;
        instruction->left = left;
        instruction->right = right;
        current_base_block->instructions.push_back(instruction);
    }

    auto jump2 = current_base_block->instruction<HIRJump>();
    jump2->label_id = end_label;
    current_base_block->instructions.push_back(jump2);

    current_base_block->instructions.push_back(
        current_base_block->instruction<HIRLabel>(false_label));

    auto instruction = current_base_block->instruction<HIROperation>();
    instruction->operation_type = HIROperationType::assign;
    instruction->result_type = convert_type_to_hir(left_type);
    instruction->operand_type = instruction->operand_type;
    instruction->target = result;
    instruction->left = left;
    current_base_block->instructions.push_back(instruction);

    current_base_block->instructions.push_back(
        current_base_block->instruction<HIRLabel>(end_label));

    push_value(result);
}

void bonk::HIREarlyGeneratorVisitor::visit(bonk::TreeNodeUnaryOperation* node) {
    ASTVisitor::visit(node);

    Type* type = middle_end.type_table.get_type(node->operand.get());

    int operand = register_stack.back().register_id;

    assert(type->kind == TypeKind::primitive);

    auto primitive = (TreeNodePrimitiveType*)type;

    assert(primitive->primitive_type == PrimitiveType::t_shrt ||
           primitive->primitive_type == PrimitiveType::t_nubr ||
           primitive->primitive_type == PrimitiveType::t_long ||
           primitive->primitive_type == PrimitiveType::t_flot ||
           primitive->primitive_type == PrimitiveType::t_dabl);

    HIRDataType hir_type = convert_type_to_hir(type);

    int zero_operand = middle_end.id_table.get_unused_id();
    HIRConstantLoad* zero_load_instruction = nullptr;

    if (primitive->primitive_type == PrimitiveType::t_nubr) {
        zero_load_instruction =
            current_base_block->instruction<HIRConstantLoad>(zero_operand, (int32_t)0);
    } else {
        zero_load_instruction =
            current_base_block->instruction<HIRConstantLoad>(zero_operand, (float)0.0f);
    }

    current_base_block->instructions.push_back(zero_load_instruction);

    int id = middle_end.id_table.get_unused_id();
    auto instruction = current_base_block->instruction<HIROperation>();
    instruction->operation_type = HIROperationType::minus;
    instruction->result_type = hir_type;
    instruction->operand_type = hir_type;
    instruction->target = id;
    instruction->left = zero_operand;
    instruction->right = operand;

    current_base_block->instructions.push_back(instruction);
    push_value(id);
}

void bonk::HIREarlyGeneratorVisitor::visit(bonk::TreeNodeHiveAccess* node) {
    node->hive->accept(this);

    auto hive = register_stack.back();
    register_stack.pop_back();

    auto hive_type = middle_end.type_table.get_type(node->hive.get());
    assert(hive_type->kind == TypeKind::hive);

    auto hive_definition = ((HiveType*)hive_type)->hive_definition;

    int field_index = 0;

    for (auto& child : hive_definition->body) {
        if (child->type == TreeNodeType::n_variable_definition) {
            auto variable = (TreeNodeVariableDefinition*)child.get();
            if (variable->variable_name->identifier_text == node->field->identifier_text)
                break;
            field_index++;
        } else if (child->type == TreeNodeType::n_block_definition) {
            auto block = (TreeNodeBlockDefinition*)child.get();
            if (block->block_name->identifier_text == node->field->identifier_text) {
                assert(!"Cannot access block methods yet");
            }
        }
    }

    int offset = middle_end.get_hive_field_offset(hive_definition, field_index);

    int constant_storage = middle_end.id_table.get_unused_id();

    auto constant_load =
        current_base_block->instruction<HIRConstantLoad>(constant_storage, (int64_t)offset);
    current_base_block->instructions.push_back(constant_load);

    int result_id = middle_end.id_table.get_unused_id();

    auto instruction = current_base_block->instruction<HIROperation>();
    instruction->operation_type = HIROperationType::plus;
    instruction->result_type = HIRDataType::dword;
    instruction->operand_type = HIRDataType::dword;
    instruction->target = result_id;
    instruction->left = load_value(hive, HIRDataType::dword);
    instruction->right = constant_storage;
    current_base_block->instructions.push_back(instruction);

    // Decrease the reference count of the hive, and increase the reference count of the field
    // if it is a hive reference

    auto field_type = middle_end.type_table.get_type(node);
    if (field_type->kind == TypeKind::hive) {
        // Since result_id is a pointer to the field, we need to load the value of the field
        int field_reference = middle_end.id_table.get_unused_id();
        auto load_instruction = current_base_block->instruction<HIRMemoryLoad>();
        load_instruction->target = field_reference;
        load_instruction->type = HIRDataType::dword;
        load_instruction->address = result_id;
        current_base_block->instructions.push_back(load_instruction);

        auto reference_increment = current_base_block->instruction<HIRIncRefCounter>();
        reference_increment->address = field_reference;
        current_base_block->instructions.push_back(reference_increment);
    }

    auto reference_decrement = current_base_block->instruction<HIRDecRefCounter>();
    reference_decrement->address = hive.register_id;
    reference_decrement->hive_definition = hive_definition;
    current_base_block->instructions.push_back(reference_decrement);

    push_reference(result_id);
}

void bonk::HIREarlyGeneratorVisitor::visit(bonk::TreeNodeBonkStatement* node) {
    ASTVisitor::visit(node);

    auto instruction = current_base_block->instruction<HIRReturn>();

    if (node->expression) {
        Type* type = middle_end.type_table.get_type(node->expression.get());
        auto id = register_stack.back();
        auto hir_type = convert_type_to_hir(type);
        register_stack.pop_back();
        instruction->return_value = load_value(id, hir_type);
        instruction->return_type = hir_type;
    }

    kill_alive_variables(current_block_definition);

    current_base_block->instructions.push_back(instruction);
}

void bonk::HIREarlyGeneratorVisitor::visit(bonk::TreeNodeLoopStatement* node) {
    alive_scopes.emplace_back();
    alive_scopes.back().block = node;

    auto old_loop_context = current_loop_context;

    if (node->loop_parameters) {
        // Setup loop variables
        node->loop_parameters->accept(this);
    }

    int loop_start_id = middle_end.id_table.get_id(node);
    int loop_end_id = middle_end.id_table.get_unused_id();

    current_loop_context = HIRLoopContext{};
    current_loop_context->loop_start_label = loop_start_id;
    current_loop_context->loop_end_label = loop_end_id;
    current_loop_context->loop_block = node;

    // Insert loop start label
    auto loop_start_label = current_base_block->instruction<HIRLabel>(loop_start_id);
    current_base_block->instructions.push_back(loop_start_label);

    // Compile loop body
    node->body->accept(this);

    // Insert jump to loop start
    auto jump_instruction = current_base_block->instruction<HIRJump>();
    jump_instruction->label_id = loop_start_id;
    current_base_block->instructions.push_back(jump_instruction);

    // Insert loop end label
    auto loop_end_label = current_base_block->instruction<HIRLabel>(loop_end_id);
    current_base_block->instructions.push_back(loop_end_label);

    current_loop_context = old_loop_context;

    alive_scopes.pop_back();
}

void bonk::HIREarlyGeneratorVisitor::visit(bonk::TreeNodeBrekStatement* node) {
    assert(current_loop_context.has_value());

    kill_alive_variables(current_loop_context->loop_block);

    auto instruction = current_base_block->instruction<HIRJump>();
    instruction->label_id = current_loop_context->loop_end_label;
    current_base_block->instructions.push_back(instruction);
}

void bonk::HIREarlyGeneratorVisitor::visit(bonk::TreeNodeHiveDefinition* node) {
    auto old_hive = current_hive_definition;
    current_hive_definition = node;
    for (auto& child : node->body) {
        child->accept(this);
    }
    current_hive_definition = old_hive;
}

void bonk::HIREarlyGeneratorVisitor::visit(bonk::TreeNodeCall* node) {

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
                if (parameter->variable_value) {
                    parameter->variable_value->accept(this);
                } else {
                    // TODO: move this check somewhere else
                    middle_end.linked_compiler.error().at(node->source_position)
                        << "Missing argument for parameter \"" << parameter_name->identifier_text
                        << "\"";
                }
            }
        }

        if(middle_end.linked_compiler.state) {
            return;
        }

        std::vector<RegisterStackItem> parameters;

        for (auto& argument : block_type->parameters) {
            auto argument_type = middle_end.type_table.get_type(argument);
            auto argument_id = register_stack.back();
            parameters.push_back(argument_id);
            register_stack.pop_back();

            auto instruction = current_base_block->instruction<HIRParameter>();
            instruction->parameter = argument_id.register_id;
            instruction->type = convert_type_to_hir(argument_type);
            current_base_block->instructions.push_back(instruction);
        }

        auto return_register = middle_end.id_table.get_unused_id();
        auto instruction = current_base_block->instruction<HIRCall>();
        instruction->procedure_label_id = label_id;
        instruction->return_value = return_register;
        instruction->return_type = convert_type_to_hir(block_type->return_type.get());
        current_base_block->instructions.push_back(instruction);

        // If some parameters were hive variables, we need to decrement their reference counters,
        // because their temporary copies are no longer alive

        int i = 0;
        for (auto& parameter : block_type->parameters) {
            auto& parameter_register = parameters[i++];

            auto argument_type = middle_end.type_table.get_type(parameter);
            if (argument_type->kind != TypeKind::hive)
                continue;

            auto hive_type = (HiveType*)argument_type;
            auto hir_type = convert_type_to_hir(hive_type);

            IRRegister hive_address = load_value(parameter_register, hir_type);

            auto ref_dec_instruction = current_base_block->instruction<HIRDecRefCounter>();
            ref_dec_instruction->address = hive_address;
            ref_dec_instruction->hive_definition = hive_type->hive_definition;
            current_base_block->instructions.push_back(ref_dec_instruction);
        }

        push_value(return_register);

    } else {
        assert(!"Cannot call function by pointer yet");
    }
}

std::unique_ptr<bonk::IRProgram> bonk::HIREarlyGeneratorVisitor::generate(bonk::TreeNode* ast) {
    auto program = std::make_unique<IRProgram>(middle_end.id_table, middle_end.symbol_table);
    current_program = program.get();
    ast->accept(this);

    HIRBaseBlockSeparator::separate_blocks(*program);
    return program;
}

bonk::HIROperationType bonk::HIREarlyGeneratorVisitor::convert_operation_to_hir(OperatorType type) {
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

bonk::HIRDataType bonk::HIREarlyGeneratorVisitor::convert_type_to_hir(bonk::Type* type) {
    switch (type->kind) {

    case TypeKind::primitive: {
        auto primitive = ((TrivialType*)type)->primitive_type;

        switch (primitive) {
        case PrimitiveType::t_strg:
            // Pointer type
            return HIRDataType::dword;
        case PrimitiveType::t_buul:
            return HIRDataType::byte;
        case PrimitiveType::t_shrt:
            return HIRDataType::hword;
        case PrimitiveType::t_nubr:
            return HIRDataType::word;
        case PrimitiveType::t_long:
            return HIRDataType::dword;
        case PrimitiveType::t_flot:
            return HIRDataType::float32;
        case PrimitiveType::t_dabl:
            return HIRDataType::float64;
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

bool bonk::HIREarlyGeneratorVisitor::is_comparison_operation(bonk::HIROperationType type) {
    switch (type) {
    case HIROperationType::equal:
    case HIROperationType::not_equal:
    case HIROperationType::less:
    case HIROperationType::less_equal:
    case HIROperationType::greater:
    case HIROperationType::greater_equal:
        return true;
    default:
        return false;
    }
}

void bonk::HIREarlyGeneratorVisitor::push_value(bonk::IRRegister register_id) {
    register_stack.push_back({register_id, false});
}

void bonk::HIREarlyGeneratorVisitor::push_reference(bonk::IRRegister register_id) {
    register_stack.push_back({register_id, true});
}

bonk::IRRegister bonk::HIREarlyGeneratorVisitor::load_value(RegisterStackItem item,
                                                            HIRDataType type) {
    if (item.is_reference) {
        int value_register = middle_end.id_table.get_unused_id();
        auto instruction = current_base_block->instruction<HIRMemoryLoad>();
        instruction->target = value_register;
        instruction->type = type;
        instruction->address = item.register_id;
        current_base_block->instructions.push_back(instruction);
        return value_register;
    } else {
        return item.register_id;
    }
}

void bonk::HIREarlyGeneratorVisitor::kill_alive_variables(TreeNode* until_scope) {
    for (int i = alive_scopes.size() - 1; i >= 0; i--) {
        auto scope = alive_scopes[i];
        for (int j = scope.alive_variables.size() - 1; j >= 0; j--) {
            handle_variable_death(scope.alive_variables[j]);
        }
        if (scope.block == until_scope)
            break;
    }
}

void bonk::HIREarlyGeneratorVisitor::handle_variable_death(
    bonk::TreeNodeVariableDefinition* variable) {

    // If the dead variable is a reference to a hive, we need to decrement the hive's reference
    // count
    auto type = middle_end.type_table.get_type(variable);
    if (type->kind == TypeKind::hive) {
        auto instruction = current_base_block->instruction<HIRDecRefCounter>();
        int variable_register = middle_end.id_table.get_id(variable);
        instruction->address = variable_register;
        instruction->hive_definition = ((HiveType*)type)->hive_definition;
        current_base_block->instructions.push_back(instruction);
    }
}
