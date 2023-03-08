
#include "type_inferring.hpp"
#include "../compiler.hpp"

void bonk::TypeInferringVisitor::visit(TreeNodeProgram* node) {
    assert(!"Cannot infer type of program");
}

void bonk::TypeInferringVisitor::visit(TreeNodeHelp* node) {
    assert(!"Cannot infer type of help statement");
}

void bonk::TypeInferringVisitor::visit(TreeNodeIdentifier* node) {
    auto def = visitor.scoped_name_resolver.get_name_definition(node->identifier_text);
    if (def == nullptr) {
        visitor.linked_compiler.error().at(node->source_position)
            << "Identifier '" << node->identifier_text << "' is not defined";
        return;
    }
    annotate(node, infer_type(def));
}

void bonk::TypeInferringVisitor::visit(TreeNodeBlockDefinition* node) {

    auto blok_type = annotate<bonk::BlokType>(node);

    if (node->block_parameters) {
        for (auto& parameter : node->block_parameters->parameters) {
            blok_type->parameters.push_back(parameter.get());
        }
    }

    if (std::find(block_stack.begin(), block_stack.end(), node) != block_stack.end()) {
        // If there is a cycle in the block graph, there is no way to return
        // from these blocks, so their type is "never"
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
        annotate(node, blok_type);
        return;
    }

    Type* return_type = nullptr;

    for (auto& bonk_statement : bonk_statements) {
        auto bonk_type = infer_type(bonk_statement);

        if(!bonk_type) continue;

        if (return_type == nullptr) {
            return_type = bonk_type;
        } else {
            if (*return_type != *bonk_type) {
                visitor.linked_compiler.error().at(bonk_statement->source_position)
                    << "bonk statement returns type " << *bonk_type
                    << ", but first bonk statement returns type " << *return_type;
            }
        }
    }

    block_stack.pop_back();
    blok_type->return_type = return_type->shallow_copy();
}

void bonk::TypeInferringVisitor::visit(TreeNodeBonkStatement* node) {
    if(node->expression) {
        annotate(node, infer_type(node->expression.get()));
    } else {
        annotate<NothingType>(node);
    }
}

void bonk::TypeInferringVisitor::visit(TreeNodeVariableDefinition* node) {
    if (node->variable_type) {
        annotate(node, infer_type(node->variable_type.get()));
    } else if (node->variable_value) {
        annotate(node, infer_type(node->variable_value.get()));
    } else {
        visitor.linked_compiler.error().at(node->source_position)
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
    assert(!"Cannot infer type of code block");
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
                    visitor.linked_compiler.error().at(node->source_position)
                        << "Array elements must be of the same type";
                    return;
                }
            }
        }
    }

    if (element_type == nullptr) {
        visitor.linked_compiler.error().at(node->source_position)
            << "Array must have at least one element to infer type";
        return;
    }

    annotate<ManyType>(node)->element_type = element_type->shallow_copy();
}

void bonk::TypeInferringVisitor::visit(TreeNodeNumberConstant* node) {
    annotate<TrivialType>(node)->primitive_type = bonk::PrimitiveType::t_flot;
}

void bonk::TypeInferringVisitor::visit(TreeNodeStringConstant* node) {
    annotate<TrivialType>(node)->primitive_type = bonk::PrimitiveType::t_strg;
}

void bonk::TypeInferringVisitor::visit(TreeNodeBinaryOperation* node) {
    auto left_type = infer_type(node->left.get());
    auto right_type = infer_type(node->right.get());

    if (!left_type->allows_binary_operation(node->operator_type, right_type)) {
        visitor.linked_compiler.error().at(node->right->source_position)
            << "Cannot perform '" << BONK_OPERATOR_NAMES[(int)node->operator_type] << "' between "
            << *left_type << " and " << *right_type;
        return;
    }

    annotate(node, left_type);
}

void bonk::TypeInferringVisitor::visit(TreeNodeUnaryOperation* node) {
    auto type = infer_type(node->operand.get());

    if (!type->allows_unary_operation(node->operator_type)) {
            visitor.linked_compiler.error().at(node->operand->source_position)
            << "Cannot perform '" << BONK_OPERATOR_NAMES[(int)node->operator_type] << "' on "
            << *type;
        return;
    }

    annotate(node, type);
}

void bonk::TypeInferringVisitor::visit(TreeNodePrimitiveType* node) {
    annotate<TrivialType>(node)->primitive_type = node->primitive_type;
}

void bonk::TypeInferringVisitor::visit(TreeNodeManyType* node) {
    annotate<ManyType>(node)->element_type = infer_type(node->parameter.get())->shallow_copy();
}

void bonk::TypeInferringVisitor::visit(TreeNodeHiveAccess* node) {
    // Determine the type of the hive
    auto type = infer_type(node->hive.get());

    if(!type || type->kind == TypeKind::error) return;

    if (type->kind != TypeKind::hive) {
        visitor.linked_compiler.error().at(node->source_position)
            << "Cannot use 'of' operator on non-hive type '" << *type << "'";
        return;
    }

    auto hive_type = (HiveType*)type;

    HiveFieldNameResolver resolver{hive_type->hive_definition};

    TreeNode* definition = resolver.get_name_definition(node->field->identifier_text);

    if (definition == nullptr) {
        visitor.linked_compiler.error().at(node->field->source_position)
            << "Cannot find hive field '" << node->field->identifier_text << "' in hive '"
            << hive_type->hive_definition->hive_name->identifier_text << "'";
        return;
    }

    annotate(node, infer_type(definition));
}

void bonk::TypeInferringVisitor::visit(TreeNodeLoopStatement* node) {
    assert(!"Cannot infer type of loop statement");
}

void bonk::TypeInferringVisitor::visit(TreeNodeHiveDefinition* node) {
    annotate<HiveType>(node)->hive_definition = node;
}

void bonk::TypeInferringVisitor::visit(TreeNodeCall* node) {
    // Don't go any deeper than this just yet
    // Determine the return type of the function
    auto type = infer_type(node->callee.get());

    if(!type || type->kind == TypeKind::error) return;

    if (type->kind != TypeKind::blok && type->kind != TypeKind::hive) {
        visitor.linked_compiler.error().at(node->callee->source_position)
            << "Cannot call non-function type";
    }

    if (type->kind == TypeKind::hive) {
        auto hive_type = (HiveType*)type;
        annotate(node, infer_type(hive_type->hive_definition));
    } else {
        auto function_type = (BlokType*)type;
        annotate(node, function_type->return_type.get());
    }
}

bonk::Type* bonk::TypeInferringVisitor::infer_type(bonk::TreeNode* node) {
    auto it = visitor.type_cache.find(node);

    if (it != visitor.type_cache.end()) {
        return it->second;
    }

    node->accept(this);

    it = visitor.type_cache.find(node);

    if(it == visitor.type_cache.end()) {
        return annotate<ErrorType>(node);
    }

    return it->second;
}

void bonk::TypeInferringVisitor::write_to_cache(TreeNode* node, Type* type) {
    visitor.type_cache.insert({node, type});
}

void bonk::TypeInferringVisitor::save_type(std::unique_ptr<Type> type) {
    visitor.type_storage.insert(std::move(type));
}
