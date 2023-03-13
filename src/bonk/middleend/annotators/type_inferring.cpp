
#include "type_inferring.hpp"
#include "../../compiler.hpp"
#include "../middleend.hpp"

void bonk::TypeInferringVisitor::visit(TreeNodeProgram* node) {
    assert(!"Cannot infer type of program");
}

void bonk::TypeInferringVisitor::visit(TreeNodeHelp* node) {
    assert(!"Cannot infer type of help statement");
}

void bonk::TypeInferringVisitor::visit(TreeNodeIdentifier* node) {
    auto def = middle_end.symbol_table.get_definition(node);
    if (def == nullptr) {
        middle_end.linked_compiler.error().at(node->source_position)
            << "Identifier '" << node->identifier_text << "' is not defined";
        return;
    }
    middle_end.type_table.annotate(node, infer_type(def));
}

void bonk::TypeInferringVisitor::visit(TreeNodeBlockDefinition* node) {

    auto blok_type = middle_end.type_table.annotate<bonk::BlokType>(node);

    if (node->block_parameters) {
        for (auto& parameter : node->block_parameters->parameters) {
            blok_type->parameters.push_back(parameter.get());
        }
    }

    if (std::find(block_stack.begin(), block_stack.end(), node) != block_stack.end()) {
        // If there is a loop in the block graph, there is no way to return
        // from these blocks, so their return type is "never"
        blok_type->return_type = std::make_unique<NeverType>();
        return;
    }
    block_stack.push_back(node);

    std::vector<bonk::TreeNodeBonkStatement*> bonk_statements;

    struct BonkVisitor : public ASTVisitor {
        std::vector<bonk::TreeNodeBonkStatement*>& bonk_statements;
        explicit BonkVisitor(std::vector<bonk::TreeNodeBonkStatement*>& bonk_statements)
            : bonk_statements(bonk_statements) {
        }

        void visit(TreeNodeBonkStatement* node) override {
            ASTVisitor::visit(node);
            bonk_statements.push_back(node);
        }
    };

    BonkVisitor bonk_visitor(bonk_statements);

    node->accept(&bonk_visitor);

    if (bonk_statements.empty()) {
        blok_type->return_type = std::make_unique<NothingType>();
        middle_end.type_table.annotate(node, blok_type);
        return;
    }

    Type* return_type = nullptr;

    for (auto& bonk_statement : bonk_statements) {
        auto bonk_type = infer_type(bonk_statement);

        if (!bonk_type)
            continue;

        if (return_type == nullptr) {
            return_type = bonk_type;
        } else {
            if (*return_type != *bonk_type) {
                middle_end.linked_compiler.error().at(bonk_statement->source_position)
                    << "bonk statement returns type " << *bonk_type
                    << ", but first bonk statement returns type " << *return_type;
            }
        }
    }

    block_stack.pop_back();
    blok_type->return_type = return_type->shallow_copy();
}

void bonk::TypeInferringVisitor::visit(TreeNodeBonkStatement* node) {
    if (node->expression) {
        middle_end.type_table.annotate(node, infer_type(node->expression.get()));
    } else {
        middle_end.type_table.annotate<NothingType>(node);
    }
}

void bonk::TypeInferringVisitor::visit(TreeNodeVariableDefinition* node) {
    if (node->variable_type) {
        middle_end.type_table.annotate(node, infer_type(node->variable_type.get()));
    } else if (node->variable_value) {
        middle_end.type_table.annotate(node, infer_type(node->variable_value.get()));
    } else {
        middle_end.linked_compiler.error().at(node->source_position)
            << "Cannot infer type of variable definition without type or value";
    }
}

void bonk::TypeInferringVisitor::visit(TreeNodeParameterListDefinition* node) {
    assert(!"Cannot infer type of parameter list definition");
}

void bonk::TypeInferringVisitor::visit(TreeNodeParameterList* node) {
    assert(!"Cannot infer type of parameter list");
}

void bonk::TypeInferringVisitor::visit(TreeNodeParameterListItem* node) {
    assert(!"Cannot infer type of parameter list item");
}

void bonk::TypeInferringVisitor::visit(TreeNodeCodeBlock* node) {
    middle_end.type_table.annotate<NothingType>(node);
}

void bonk::TypeInferringVisitor::visit(TreeNodeArrayConstant* node) {
    // Make sure all elements are of the same type
    Type* element_type = nullptr;

    for (auto& element : node->elements) {
        if (element) {
            auto type = infer_type(element.get());
            if (element_type == nullptr) {
                element_type = type;
            } else {
                if (*element_type != *type) {
                    middle_end.linked_compiler.error().at(node->source_position)
                        << "Array elements must be of the same type";
                    return;
                }
            }
        }
    }

    if (element_type == nullptr) {
        middle_end.linked_compiler.error().at(node->source_position)
            << "Array must have at least one element to infer type";
        return;
    }

    middle_end.type_table.annotate<ManyType>(node)->element_type = element_type->shallow_copy();
}

void bonk::TypeInferringVisitor::visit(TreeNodeNumberConstant* node) {
    middle_end.type_table.annotate<TrivialType>(node)->primitive_type = bonk::PrimitiveType::t_flot;
}

void bonk::TypeInferringVisitor::visit(TreeNodeStringConstant* node) {
    middle_end.type_table.annotate<TrivialType>(node)->primitive_type = bonk::PrimitiveType::t_strg;
}

void bonk::TypeInferringVisitor::visit(TreeNodeBinaryOperation* node) {
    auto left_type = infer_type(node->left.get());
    auto right_type = infer_type(node->right.get());

    if (left_type->kind == TypeKind::error || right_type->kind == TypeKind::error)
        return;

    if (!left_type->allows_binary_operation(node->operator_type, right_type)) {
        middle_end.linked_compiler.error().at(node->right->source_position)
            << "Cannot perform '" << BONK_OPERATOR_NAMES[(int)node->operator_type] << "' between "
            << *left_type << " and " << *right_type;
        return;
    }

    switch (node->operator_type) {

    case OperatorType::o_plus:
    case OperatorType::o_minus:
    case OperatorType::o_multiply:
    case OperatorType::o_divide:
    case OperatorType::o_plus_assign:
    case OperatorType::o_minus_assign:
    case OperatorType::o_multiply_assign:
    case OperatorType::o_divide_assign:
    case OperatorType::o_assign:
    case OperatorType::o_and:
    case OperatorType::o_or:
        middle_end.type_table.annotate(node, left_type);
        break;
    case OperatorType::o_equal:
    case OperatorType::o_less:
    case OperatorType::o_greater:
    case OperatorType::o_less_equal:
    case OperatorType::o_greater_equal:
    case OperatorType::o_not_equal:
        middle_end.type_table.annotate<TrivialType>(node)->primitive_type =
            bonk::PrimitiveType::t_buul;
        return;
    default:
        assert(!"Unsupported binary operator");
    }
}

void bonk::TypeInferringVisitor::visit(TreeNodeUnaryOperation* node) {
    auto type = infer_type(node->operand.get());

    if (!type->allows_unary_operation(node->operator_type)) {
        middle_end.linked_compiler.error().at(node->operand->source_position)
            << "Cannot perform '" << BONK_OPERATOR_NAMES[(int)node->operator_type] << "' on "
            << *type;
        return;
    }

    middle_end.type_table.annotate(node, type);
}

void bonk::TypeInferringVisitor::visit(TreeNodePrimitiveType* node) {
    middle_end.type_table.annotate<TrivialType>(node)->primitive_type = node->primitive_type;
}

void bonk::TypeInferringVisitor::visit(TreeNodeManyType* node) {
    middle_end.type_table.annotate<ManyType>(node)->element_type =
        infer_type(node->parameter.get())->shallow_copy();
}

void bonk::TypeInferringVisitor::visit(TreeNodeHiveAccess* node) {
    // Determine the type of the hive
    auto type = infer_type(node->hive.get());

    if (!type || type->kind == TypeKind::error)
        return;

    if (type->kind != TypeKind::hive) {
        middle_end.linked_compiler.error().at(node->source_position)
            << "Cannot use 'of' operator on non-hive type '" << *type << "'";
        return;
    }

    auto hive_type = (HiveType*)type;

    HiveFieldNameResolver resolver{hive_type->hive_definition};

    TreeNode* definition = resolver.get_name_definition(node->field->identifier_text);

    if (definition == nullptr) {
        middle_end.linked_compiler.error().at(node->field->source_position)
            << "Cannot find hive field '" << node->field->identifier_text << "' in hive '"
            << hive_type->hive_definition->hive_name->identifier_text << "'";
        return;
    }

    middle_end.type_table.annotate(node, infer_type(definition));
}

void bonk::TypeInferringVisitor::visit(TreeNodeLoopStatement* node) {
    assert(!"Cannot infer type of loop statement");
}

void bonk::TypeInferringVisitor::visit(TreeNodeHiveDefinition* node) {
    middle_end.type_table.annotate<HiveType>(node)->hive_definition = node;
}

void bonk::TypeInferringVisitor::visit(TreeNodeCall* node) {

    // Determine the return type of the function
    auto callee_type = infer_type(node->callee.get());

    if (!callee_type || callee_type->kind == TypeKind::error)
        return;

    if (callee_type->kind != TypeKind::blok) {
        middle_end.linked_compiler.error().at(node->callee->source_position)
            << "Cannot call non-function type";
        return;
    }

    auto function_type = (BlokType*)callee_type;
    middle_end.type_table.annotate(node, function_type->return_type.get());

    // Infer type of expressions in arguments
    // and check if they are compatible with function arguments
    if (!node->arguments) return;

    for (auto& argument : node->arguments->parameters) {
        Type* argument_type = infer_type(argument->parameter_value.get());

        if (argument_type->kind == TypeKind::error)
            continue;

        // Find parameter named like argument and check if it is compatible with argument
        // type
        FunctionParameterNameResolver resolver{function_type};

        auto definition = resolver.get_name_definition(argument->parameter_name->identifier_text);

        if (definition == nullptr) {
            middle_end.linked_compiler.error().at(argument->parameter_name->source_position)
                << "Cannot find function parameter '"
                << argument->parameter_name->identifier_text << "' in function '"
                << *function_type << "'";
            continue;
        }

        middle_end.symbol_table.symbol_definitions[argument->parameter_name.get()] = definition;

        Type* valid_type = infer_type(definition);

        if (valid_type->kind == TypeKind::error)
            continue;

        if (*valid_type != *argument_type) {
            middle_end.linked_compiler.error().at(argument->parameter_value->source_position)
                << "Cannot pass '" << *argument_type << "' to parameter '"
                << argument->parameter_name->identifier_text << "' of type '" << *valid_type
                << "'";
        }
    }
}

bonk::Type* bonk::TypeInferringVisitor::infer_type(bonk::TreeNode* node) {
    auto& type_table = middle_end.type_table;
    auto it = type_table.type_cache.find(node);

    if (it != type_table.type_cache.end()) {
        return it->second;
    }

    node->accept(this);

    it = type_table.type_cache.find(node);

    if (it == type_table.type_cache.end()) {
        return type_table.annotate<ErrorType>(node);
    }

    return it->second;
}
