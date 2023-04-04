
#include "hir_early_generator_visitor.hpp"
#include "bonk/middleend/annotators/basic_symbol_annotator.hpp"
#include "bonk/middleend/annotators/type_visitor.hpp"
#include "hir.hpp"
#include "hir_base_block_separator.hpp"

void bonk::HIREarlyGeneratorVisitor::visit(bonk::TreeNodeProgram* node) {
    ASTVisitor::visit(node);
}

void bonk::HIREarlyGeneratorVisitor::visit(bonk::TreeNodeHelp* node) {
}

void bonk::HIREarlyGeneratorVisitor::visit(bonk::TreeNodeIdentifier* node) {
    write_location(node);

    // If identifier holds reference to a hive, increase reference counter

    auto id = middle_end.id_table.get_id(node);
    auto type = middle_end.type_table.get_type(node);

    auto value = std::make_unique<HIRValue>(*this);
    value->set_value(id, type);
    value->increase_reference_counter();

    return_value = std::move(value);
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

    write_file(node);

    int id = middle_end.id_table.get_id(node);
    auto type = (BlokType*)middle_end.type_table.get_type(node);
    HIRDataType return_type = convert_type_to_hir(type->return_type.get());

    // Create a procedure instruction
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

    // Create parameter values (if it's necessary)
    if (node->block_parameters && node->body) {
        for (auto& parameter : node->block_parameters->parameters) {
            if (parameter) {
                int parameter_id = middle_end.id_table.get_id(parameter.get());
                auto parameter_type = (BlokType*)middle_end.type_table.get_type(parameter.get());

                auto value = std::make_unique<HIRValue>(*this);
                value->set_value(parameter_id, parameter_type);
                value->increase_reference_counter();

                alive_scopes.back().alive_values.push_back(std::move(value));
            }
        }
    }

    // Compile body
    if (node->body) {
        node->body->accept(this);

        current_block_definition = old_block;
        alive_scopes.pop_back();

        auto instruction = current_base_block->instruction<HIRReturn>();
        current_base_block->instructions.push_back(instruction);
    } else {
        procedure_header->is_external = true;
        current_block_definition = old_block;
        alive_scopes.pop_back();
    }
}

void bonk::HIREarlyGeneratorVisitor::visit(bonk::TreeNodeVariableDefinition* node) {
    write_location(node);

    if (!current_block_definition) {
        // It could be a global variable or a field of a hive
        return;
    }

    // Create new HIRValue for variable

    int variable_id = middle_end.id_table.get_id(node);
    auto variable_value = std::make_unique<HIRValue>(*this);
    auto variable_type = middle_end.type_table.get_type(node);

    // Initialize variable with zero
    // This is needed because its type could be a hive. Assignment
    // operation decreases the reference counter of the left-hand operand
    // if pointer is not null, so it should be set to null explicitly

    auto instruction = current_base_block->instruction<HIRConstantLoad>(variable_id, (int64_t)0);
    current_base_block->instructions.push_back(instruction);

    variable_value->set_value(variable_id, variable_type);
    variable_value->increase_reference_counter();

    auto variable_raw = variable_value.get();

    alive_scopes.back().alive_values.emplace_back(std::move(variable_value));

    if (node->variable_value == nullptr)
        return;

    // Get the result of the right part of the assignment
    auto result = eval(node->variable_value.get());

    assign(variable_raw, result.get());
}

void bonk::HIREarlyGeneratorVisitor::visit(bonk::TreeNodeParameterList* node) {
    ASTVisitor::visit(node);
}

void bonk::HIREarlyGeneratorVisitor::visit(bonk::TreeNodeParameterListItem* node) {
    ASTVisitor::visit(node);
}

void bonk::HIREarlyGeneratorVisitor::visit(bonk::TreeNodeCodeBlock* node) {
    write_location(node);

    alive_scopes.emplace_back();
    alive_scopes.back().block = node;

    for (auto& element : node->body) {
        if (element) {
            element->accept(this);
            return_value = nullptr;
        }
    }

    alive_scopes.pop_back();
}

void bonk::HIREarlyGeneratorVisitor::visit(bonk::TreeNodeArrayConstant* node) {
    write_location(node);
    assert(!"Cannot compile this just yet");
}

void bonk::HIREarlyGeneratorVisitor::visit(bonk::TreeNodeNumberConstant* node) {
    write_location(node);

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

    auto value = std::make_unique<HIRValue>(*this);
    value->set_value(id, type);
    return_value = std::move(value);
}

void bonk::HIREarlyGeneratorVisitor::visit(bonk::TreeNodeStringConstant* node) {
    write_location(node);

    int id = middle_end.id_table.get_unused_id();

    // TODO: find the string in the string table and get its pointer
    int64_t string_pointer = 0;
    auto instruction = current_base_block->instruction<HIRConstantLoad>(id, string_pointer);
    auto type = middle_end.type_table.get_type(node);
    current_base_block->instructions.push_back(instruction);

    auto value = std::make_unique<HIRValue>(*this);
    value->set_value(id, type);
    value->increase_reference_counter();
    return_value = std::move(value);
}

std::unique_ptr<bonk::HIRValue> bonk::HIREarlyGeneratorVisitor::assign(HIRValue* left,
                                                                       HIRValue* right) {

    HIRDataType hir_type = convert_type_to_hir(left->get_type());

    auto right_loaded = load_value(right);

    if (left->is_reference()) {
        auto instruction = current_base_block->instruction<HIRMemoryStore>();
        instruction->address = std::get<HIRValueReference>(left->value).register_id;
        instruction->value = std::get<HIRValueRaw>(right_loaded->value).register_id;
        instruction->type = hir_type;
        current_base_block->instructions.push_back(instruction);
    } else {
        // Transfer the reference count, because the left-hand operand will be
        // overwritten with the right-hand operand. This is not the case
        // if the left-hand operand is a reference.

        right_loaded->increase_reference_counter();
        load_value(left)->decrease_reference_count();

        auto instruction = current_base_block->instruction<HIROperation>();
        instruction->operation_type = HIROperationType::assign;
        instruction->operand_type = hir_type;
        instruction->result_type = hir_type;
        instruction->target = std::get<HIRValueRaw>(left->value).register_id;
        instruction->left = std::get<HIRValueRaw>(right_loaded->value).register_id;
        current_base_block->instructions.push_back(instruction);
    }

    return right_loaded;
}

void bonk::HIREarlyGeneratorVisitor::visit(bonk::TreeNodeBinaryOperation* node) {
    write_location(node);

    if (node->operator_type == OperatorType::o_and || node->operator_type == OperatorType::o_or) {
        compile_lazy_logic(node);
        return;
    }

    Type* left_type = middle_end.type_table.get_type(node->left.get());
    Type* right_type = middle_end.type_table.get_type(node->right.get());

    auto left = eval(node->left.get());
    auto right = eval(node->right.get());

    assert(left_type->kind != TypeKind::unset && right_type->kind != TypeKind::unset);

    HIROperationType operation_type = convert_operation_to_hir(node->operator_type);

    if (operation_type == HIROperationType::assign) {
        // Transfer the reference count (because right value will be returned)
        load_value(right.get())->increase_reference_counter();
        load_value(left.get())->decrease_reference_count();

        return_value = assign(left.get(), right.get());
    } else {
        HIRDataType hir_type = convert_type_to_hir(left_type);
        HIRDataType result_type = convert_type_to_hir(middle_end.type_table.get_type(node));

        int id = middle_end.id_table.get_unused_id();

        auto left_loaded = load_value(left.get());
        auto right_loaded = load_value(right.get());

        auto instruction = current_base_block->instruction<HIROperation>();
        instruction->operation_type = operation_type;
        instruction->operand_type = hir_type;
        instruction->result_type = result_type;
        instruction->target = id;
        instruction->left = std::get<HIRValueRaw>(left_loaded->value).register_id;
        instruction->right = std::get<HIRValueRaw>(right_loaded->value).register_id;

        auto result_value = std::make_unique<HIRValue>(*this);
        result_value->set_value(id, middle_end.type_table.get_type(node));
        return_value = std::move(result_value);

        current_base_block->instructions.push_back(instruction);
    }
}

void bonk::HIREarlyGeneratorVisitor::compile_lazy_logic(TreeNodeBinaryOperation* node) {
    write_location(node);

    Type* left_type = middle_end.type_table.get_type(node->left.get());
    Type* right_type = middle_end.type_table.get_type(node->right.get());

    assert(left_type->kind == TypeKind::primitive);
    assert(right_type->kind == TypeKind::primitive);

    // Compile left part, and only compile right part if it's needed
    auto left = eval(node->left.get());
    auto left_loaded = load_value(left.get());
    auto left_id = std::get<HIRValueRaw>(left_loaded->value).register_id;

    int true_label = middle_end.id_table.get_unused_id();
    int false_label = middle_end.id_table.get_unused_id();
    int end_label = middle_end.id_table.get_unused_id();

    HIRJumpNZ* jump1 = nullptr;

    if (node->operator_type == OperatorType::o_and) {
        jump1 = current_base_block->instruction<HIRJumpNZ>(left_id, true_label, false_label);
    } else {
        jump1 = current_base_block->instruction<HIRJumpNZ>(left_id, false_label, true_label);
    }

    int result = middle_end.id_table.get_unused_id();

    current_base_block->instructions.push_back(jump1);
    current_base_block->instructions.push_back(
        current_base_block->instruction<HIRLabel>(true_label));

    node->right->accept(this);

    //    if (right_type->is(TrivialTypeKind::t_nothing)) {
    // The or/and construction should work like an 'if' statement, like this:
    // ((cond and true_branch) or false_branch)
    // So, if cond turned out to be false, the value of the (cond and true_branch)
    // expression should be false, and vice versa.

    // Move left to result
    auto instruction = current_base_block->instruction<HIROperation>();
    instruction->operation_type = HIROperationType::assign;
    instruction->result_type = convert_type_to_hir(left_type);
    instruction->operand_type = instruction->operand_type;
    instruction->target = result;
    instruction->left = left_id;
    current_base_block->instructions.push_back(instruction);
    //    } else {
    //        // If it's an ordinary or/and construction, its value
    //        // should actually be calculated.
    //        auto right = eval(node->right.get());
    //        auto right_loaded = load_value(right.get());
    //        auto right_id = std::get<HIRValueRaw>(right_loaded->value).register_id;
    //
    //        HIRDataType hir_type = convert_type_to_hir(left_type);
    //        HIROperationType operation_type = convert_operation_to_hir(node->operator_type);
    //
    //        auto instruction = current_base_block->instruction<HIROperation>();
    //        instruction->operation_type = operation_type;
    //        instruction->result_type = hir_type;
    //        instruction->operand_type = hir_type;
    //        instruction->target = result;
    //        instruction->left = left_id;
    //        instruction->right = right_id;
    //        current_base_block->instructions.push_back(instruction);
    //    }

    auto jump2 = current_base_block->instruction<HIRJump>();
    jump2->label_id = end_label;
    current_base_block->instructions.push_back(jump2);

    current_base_block->instructions.push_back(
        current_base_block->instruction<HIRLabel>(false_label));

    auto assign_instruction = current_base_block->instruction<HIROperation>();
    assign_instruction->operation_type = HIROperationType::assign;
    assign_instruction->result_type = convert_type_to_hir(left_type);
    assign_instruction->operand_type = assign_instruction->operand_type;
    assign_instruction->target = result;
    assign_instruction->left = left_id;
    current_base_block->instructions.push_back(assign_instruction);

    current_base_block->instructions.push_back(
        current_base_block->instruction<HIRLabel>(end_label));

    auto result_value = std::make_unique<HIRValue>(*this);
    result_value->set_value(result, middle_end.type_table.get_type(node));
    result_value->increase_reference_counter();
    return_value = std::move(result_value);
}

void bonk::HIREarlyGeneratorVisitor::visit(bonk::TreeNodeUnaryOperation* node) {
    write_location(node);

    Type* type = middle_end.type_table.get_type(node->operand.get());

    auto operand = eval(node->operand.get());
    auto operand_loaded = load_value(operand.get());
    auto operand_id = std::get<HIRValueRaw>(operand_loaded->value).register_id;

    assert(type->kind == TypeKind::primitive);

    auto primitive = (TreeNodePrimitiveType*)type;

    assert(primitive->primitive_type == TrivialTypeKind::t_shrt ||
           primitive->primitive_type == TrivialTypeKind::t_nubr ||
           primitive->primitive_type == TrivialTypeKind::t_long ||
           primitive->primitive_type == TrivialTypeKind::t_flot ||
           primitive->primitive_type == TrivialTypeKind::t_dabl);

    HIRDataType hir_type = convert_type_to_hir(type);

    int zero_operand = middle_end.id_table.get_unused_id();
    HIRConstantLoad* zero_load_instruction = nullptr;

    if (primitive->primitive_type == TrivialTypeKind::t_nubr) {
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
    instruction->right = operand_id;

    current_base_block->instructions.push_back(instruction);

    auto result_value = std::make_unique<HIRValue>(*this);
    result_value->set_value(id, middle_end.type_table.get_type(node));
    result_value->increase_reference_counter();
    return_value = std::move(result_value);
}

void bonk::HIREarlyGeneratorVisitor::visit(bonk::TreeNodeHiveAccess* node) {
    write_location(node);

    auto hive = eval(node->hive.get());
    auto hive_loaded = load_value(hive.get());

    auto hive_type = middle_end.type_table.get_type(node->hive.get());

    if (hive_type->kind == TypeKind::external) {
        auto external_type = (ExternalType*)hive_type;
        hive_type = external_type->get_resolved();
    }

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
    instruction->left = std::get<HIRValueRaw>(hive_loaded->value).register_id;
    instruction->right = constant_storage;
    current_base_block->instructions.push_back(instruction);

    auto field_type = middle_end.type_table.get_type(node);

    // For constructions like
    // it = next of it;
    // `it` gets changed before result_value is destroyed,
    // so reference counter would decrement on the `next of it`
    // instead if `it`. This is why hive_loaded->value should
    // be copied to a separate variable.
    // Test TestHive4 in test suite TestQBEFullCycle
    // checks this case.

    int hive_loaded_copy_id = middle_end.id_table.get_unused_id();

    auto hive_loaded_copy_instruction = current_base_block->instruction<HIROperation>();
    hive_loaded_copy_instruction->operation_type = HIROperationType::assign;
    hive_loaded_copy_instruction->result_type = HIRDataType::dword;
    hive_loaded_copy_instruction->operand_type = HIRDataType::dword;
    hive_loaded_copy_instruction->target = hive_loaded_copy_id;
    hive_loaded_copy_instruction->left = std::get<HIRValueRaw>(hive_loaded->value).register_id;
    current_base_block->instructions.push_back(hive_loaded_copy_instruction);

    auto hive_loaded_copy = std::make_unique<HIRValue>(*this);
    hive_loaded_copy->set_value(hive_loaded_copy_id, hive_type);
    hive_loaded_copy->increase_reference_counter();

    // hive_loaded_copy is going to be destroyed after this function,
    // but its value is still used by the result_value. Its reference counter will
    // be decremented in the hive_loaded_copy destructor.

    auto result_value = std::make_unique<HIRValue>(*this);
    result_value->set_reference(result_id, std::get<HIRValueRaw>(hive_loaded_copy->value),
                                field_type);
    result_value->increase_reference_counter();

    return_value = std::move(result_value);
}

void bonk::HIREarlyGeneratorVisitor::visit(bonk::TreeNodeBonkStatement* node) {
    write_location(node);

    auto instruction = current_base_block->instruction<HIRReturn>();

    // The expression (if it's present) should outlive the instruction,
    // so reference counter is not decremented before returning.
    // This is why expression is not moved in the if statement.

    std::unique_ptr<bonk::HIRValue> expression_loaded;

    if (node->expression) {
        Type* type = middle_end.type_table.get_type(node->expression.get());
        auto expression = eval(node->expression.get());
        expression_loaded = load_value(expression.get());

        instruction->return_value = std::get<HIRValueRaw>(expression_loaded->value).register_id;
        instruction->return_type = convert_type_to_hir(type);
    }

    kill_alive_variables(current_block_definition);
    current_base_block->instructions.push_back(instruction);
}

void bonk::HIREarlyGeneratorVisitor::visit(bonk::TreeNodeLoopStatement* node) {
    write_location(node);
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

    current_loop_context = old_loop_context;
    alive_scopes.pop_back();

    // Insert loop end label (after killing all variables, including loop variables)
    auto loop_end_label = current_base_block->instruction<HIRLabel>(loop_end_id);
    current_base_block->instructions.push_back(loop_end_label);
}

void bonk::HIREarlyGeneratorVisitor::visit(bonk::TreeNodeBrekStatement* node) {
    write_location(node);
    assert(current_loop_context.has_value());

    kill_alive_variables(current_loop_context->loop_block);

    auto instruction = current_base_block->instruction<HIRJump>();
    instruction->label_id = current_loop_context->loop_end_label;
    current_base_block->instructions.push_back(instruction);
}

void bonk::HIREarlyGeneratorVisitor::visit(bonk::TreeNodeHiveDefinition* node) {
    write_location(node);
    auto old_hive = current_hive_definition;
    current_hive_definition = node;
    for (auto& child : node->body) {
        child->accept(this);
    }
    current_hive_definition = old_hive;
}

void bonk::HIREarlyGeneratorVisitor::visit(bonk::TreeNodeCall* node) {
    write_location(node);
    auto callee = node->callee.get();

    if (callee->type != TreeNodeType::n_identifier) {
        assert(!"Cannot call function by pointer yet");
    }

    auto identifier = (TreeNodeIdentifier*)callee;

    // Called function must be defined locally at this point, otherwise it is an error
    auto definition = middle_end.symbol_table.symbol_definitions[identifier].get_local().definition;
    auto identifier_type = middle_end.type_table.get_type(definition);

    assert(identifier_type->kind == TypeKind::blok);

    auto block_type = (BlokType*)identifier_type;

    int label_id = middle_end.id_table.get_id(definition);

    std::vector<std::unique_ptr<HIRValue>> parameters;

    for (auto& parameter : block_type->parameters) {
        auto& parameter_name = parameter->variable_name;

        TreeNode* parameter_node = nullptr;

        if (node->arguments) {
            for (auto& argument : node->arguments->parameters) {
                if (argument->parameter_name->identifier_text == parameter_name->identifier_text) {
                    parameter_node = argument->parameter_value.get();
                    break;
                }
            }
        }

        std::unique_ptr<HIRValue> parameter_value;

        if (parameter_node)
            parameter_value = eval(parameter_node);
        else if (parameter->variable_value)
            parameter_value = eval(parameter->variable_value.get());

        if (parameter_value) {
            auto parameter_loaded = load_value(parameter_value.get());
            parameters.push_back(std::move(parameter_loaded));
            continue;
        }

        // TODO: move this diagnostic somewhere else
        middle_end.linked_compiler.error().at(node->source_position)
            << "Missing argument for parameter \"" << parameter_name->identifier_text << "\"";
    }

    if (middle_end.linked_compiler.state) {
        return;
    }

    for (auto& parameter : parameters) {
        auto instruction = current_base_block->instruction<HIRParameter>();
        instruction->parameter = std::get<HIRValueRaw>(parameter->value).register_id;
        instruction->type = convert_type_to_hir(parameter->get_type());
        current_base_block->instructions.push_back(instruction);
    }

    auto return_register = middle_end.id_table.get_unused_id();
    auto instruction = current_base_block->instruction<HIRCall>();
    instruction->procedure_label_id = label_id;
    instruction->return_value = return_register;
    instruction->return_type = convert_type_to_hir(block_type->return_type.get());
    current_base_block->instructions.push_back(instruction);

    auto result = std::make_unique<HIRValue>(*this);
    result->set_value(return_register, block_type->return_type.get());
    return_value = std::move(result);
}

void bonk::HIREarlyGeneratorVisitor::visit(TreeNodeCast* node) {
    write_location(node);
    auto value = eval(node->operand.get());

    // For now, casts are only used to cast between long and hive types,
    // They are compiler-generated, so we don't need to validate them

    auto type = middle_end.type_table.get_type(node);

    if (value->is_reference()) {
        std::get<HIRValueReference>(value->value).type = type;
    } else {
        std::get<HIRValueRaw>(value->value).type = type;
    }

    return_value = std::move(value);
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
    if (type->kind == TypeKind::external) {
        auto external_type = (ExternalType*)type;
        type = external_type->get_resolved();
    }

    switch (type->kind) {

    case TypeKind::primitive: {
        auto primitive = ((TrivialType*)type)->trivial_kind;

        switch (primitive) {
        case TrivialTypeKind::t_strg:
            // Pointer type
            return HIRDataType::dword;
        case TrivialTypeKind::t_buul:
            return HIRDataType::byte;
        case TrivialTypeKind::t_shrt:
            return HIRDataType::hword;
        case TrivialTypeKind::t_nubr:
            return HIRDataType::word;
        case TrivialTypeKind::t_long:
            return HIRDataType::dword;
        case TrivialTypeKind::t_flot:
            return HIRDataType::float32;
        case TrivialTypeKind::t_dabl:
            return HIRDataType::float64;
        case TrivialTypeKind::t_nothing:
            return HIRDataType::word;
        default:
            assert(false);
        }
    }
    case TypeKind::null:
    case TypeKind::hive:
    case TypeKind::many:
    case TypeKind::blok:
        // Pointer type
        return HIRDataType::dword;

    default:
        assert(!"Unsupported type");
    }
}

std::unique_ptr<bonk::HIRValue> bonk::HIREarlyGeneratorVisitor::load_value(HIRValue* value) {
    if (value->is_reference()) {
        int value_register = middle_end.id_table.get_unused_id();
        auto instruction = current_base_block->instruction<HIRMemoryLoad>();
        instruction->target = value_register;
        instruction->type = convert_type_to_hir(value->get_type());
        instruction->address = std::get<HIRValueReference>(value->value).register_id;
        current_base_block->instructions.push_back(instruction);

        auto result = std::make_unique<HIRValue>(*this);
        result->set_value(value_register, value->get_type());
        result->increase_reference_counter();
        return result;
    } else {
        auto result = std::make_unique<HIRValue>(*this);
        result->set_value(std::get<HIRValueRaw>(value->value).register_id, value->get_type());
        result->increase_reference_counter();
        return result;
    }
}

void bonk::HIREarlyGeneratorVisitor::kill_alive_variables(TreeNode* until_scope) {
    for (int i = alive_scopes.size() - 1; i >= 0; i--) {
        auto& scope = alive_scopes[i];
        for (int j = scope.alive_values.size() - 1; j >= 0; j--) {
            scope.alive_values[j]->decrease_reference_count();
        }
        if (scope.block == until_scope)
            break;
    }
}

std::unique_ptr<bonk::HIRValue> bonk::HIREarlyGeneratorVisitor::eval(bonk::TreeNode* node) {
    node->accept(this);
    return std::move(return_value);
}

void bonk::HIREarlyGeneratorVisitor::write_file(bonk::TreeNode* operation) {
    auto location = operation->source_position;
    if (current_base_block && location.line > 0 && location.ch > 0) {
        auto instruction = current_base_block->instruction<HIRFile>();
        instruction->file = location.filename;
        current_base_block->instructions.push_back(instruction);
    }
}

void bonk::HIREarlyGeneratorVisitor::write_location(bonk::TreeNode* operation) {
    auto location = operation->source_position;
    if (current_base_block && location.line > 0 && location.ch > 0) {
        auto instruction = current_base_block->instruction<HIRLocation>();
        instruction->column = location.ch;
        instruction->line = location.line;
        current_base_block->instructions.push_back(instruction);
    }
}

void bonk::HIREarlyGeneratorVisitor::visit(bonk::TreeNodeNull* node) {
    write_location(node);
    auto null_id = middle_end.id_table.get_unused_id();
    auto instruction = current_base_block->instruction<HIRConstantLoad>(null_id, (int64_t)0);
    current_base_block->instructions.push_back(instruction);

    auto null_value = std::make_unique<HIRValue>(*this);
    auto null_type = middle_end.type_table.get_type(node);
    null_value->set_value(null_id, null_type);
    // Unnecessary here
    // null_value->increase_reference_counter();
    return_value = std::move(null_value);
}

void bonk::HIRValue::increase_reference_counter() {
    if (std::holds_alternative<HIRValueReference>(value)) {
        auto& reference_container = std::get<HIRValueReference>(value).reference_container;

        auto reference_increment = visitor.current_base_block->instruction<HIRIncRefCounter>();
        reference_increment->address = reference_container.register_id;
        visitor.current_base_block->instructions.push_back(reference_increment);
    }

    if (std::holds_alternative<HIRValueRaw>(value)) {
        auto& raw_value = std::get<HIRValueRaw>(value);
        if (raw_value.type->kind != TypeKind::hive) {
            return;
        }
        auto reference_increment = visitor.current_base_block->instruction<HIRIncRefCounter>();
        reference_increment->address = raw_value.register_id;
        visitor.current_base_block->instructions.push_back(reference_increment);
    }
}

void bonk::HIRValue::decrease_reference_count() {
    if (std::holds_alternative<HIRValueReference>(value)) {
        auto& reference_container = std::get<HIRValueReference>(value).reference_container;

        auto reference_decrement = visitor.current_base_block->instruction<HIRDecRefCounter>();
        reference_decrement->address = reference_container.register_id;
        reference_decrement->hive_definition =
            ((HiveType*)reference_container.type)->hive_definition;
        visitor.current_base_block->instructions.push_back(reference_decrement);
    }

    if (std::holds_alternative<HIRValueRaw>(value)) {
        auto& raw_value = std::get<HIRValueRaw>(value);
        if (raw_value.type->kind != TypeKind::hive) {
            return;
        }
        auto reference_decrement = visitor.current_base_block->instruction<HIRDecRefCounter>();
        reference_decrement->address = raw_value.register_id;
        reference_decrement->hive_definition = ((HiveType*)get_type())->hive_definition;
        visitor.current_base_block->instructions.push_back(reference_decrement);
    }
}

bonk::Type* bonk::HIRValue::get_type() {
    if (is_reference()) {
        return std::get<HIRValueReference>(value).type;
    } else {
        return std::get<HIRValueRaw>(value).type;
    }
}

void bonk::HIRValue::set_reference(bonk::IRRegister register_id,
                                   bonk::HIRValueRaw reference_container, bonk::Type* type) {
    // Resolve types right away. This is a good place to do it here,
    // because HIR generator only works with HIRValues
    if (type->kind == TypeKind::external) {
        type = ((ExternalType*)type)->get_resolved();
    }

    value = HIRValueReference{register_id, reference_container, type};
}

void bonk::HIRValue::set_value(bonk::IRRegister register_id, bonk::Type* type) {
    // Same note here
    if (type->kind == TypeKind::external) {
        type = ((ExternalType*)type)->get_resolved();
    }

    value = HIRValueRaw{register_id, type};
}

void bonk::HIRValue::release() {
    value = std::monostate();
}

bool bonk::HIRValue::is_reference() const {
    return std::holds_alternative<HIRValueReference>(value);
}
