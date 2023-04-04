
#include "type_inferring.hpp"
#include "../../compiler.hpp"
#include "../middleend.hpp"
#include "bonk/help-resolver/help_resolver.hpp"
#include "type_visitor.hpp"

void bonk::TypeInferringVisitor::visit(TreeNodeProgram* node) {
    assert(!"Cannot infer type of program");
}

void bonk::TypeInferringVisitor::visit(TreeNodeHelp* node) {
    assert(!"Cannot infer type of help statement");
}

struct ExternalFileIdentifierResolver : bonk::ExternalTypeResolver {
    bonk::MiddleEnd& middle_end;
    std::string_view file;
    std::string_view identifier;

    ExternalFileIdentifierResolver(bonk::MiddleEnd& middle_end, std::string_view file,
                                   std::string_view identifier)
        : middle_end(middle_end), file(file), identifier(identifier) {
    }

    std::unique_ptr<bonk::Type> resolve() override {
        if (!middle_end.module_path.has_value()) {
            middle_end.linked_compiler.error()
                << "External types cannot be resolved without a module path";
            return nullptr;
        }

        auto dependency_path = middle_end.module_path->parent_path() /= file;
        auto absolute_path = std::filesystem::absolute(dependency_path);
        auto module = middle_end.get_external_module(dependency_path);
        if (!module) {
            bonk::HelpResolver resolver(middle_end.linked_compiler);
            auto metadata = resolver.get_recent_metadata_for_source(absolute_path);

            if (!metadata) {
                return nullptr;
            }

            metadata->fill_external_symbol_table(middle_end);
            auto ast = std::move(*metadata).to_ast();
            module = middle_end.add_external_module(file, std::move(ast), false);
        }

        bonk::ScopedNameResolver name_resolver;
        name_resolver.current_scope = module->scope;
        auto definition = name_resolver.get_name_definition(identifier);

        assert(definition);

        auto type = middle_end.type_table.get_type(definition);
        return bonk::TypeCloner().clone(type);
    }

    std::unique_ptr<ExternalTypeResolver> clone() override {
        return std::make_unique<ExternalFileIdentifierResolver>(middle_end, file, identifier);
    }
};

void bonk::TypeInferringVisitor::visit(TreeNodeIdentifier* node) {
    auto def = middle_end.symbol_table.get_definition(node);
    if (!def) {
        middle_end.linked_compiler.error().at(node->source_position)
            << "Identifier '" << node->identifier_text << "' is not defined";
        return;
    }
    if (def.is_local()) {
        get_current_type_table().annotate(node, infer_type(def.get_local().definition));
    }
    if (def.is_external()) {
        auto type = get_current_type_table().create<bonk::ExternalType>();

        type->resolver = std::make_unique<ExternalFileIdentifierResolver>(
            middle_end, def.get_external().file, node->identifier_text);

        get_current_type_table().annotate(node, type);
    }
}

void bonk::TypeInferringVisitor::visit(TreeNodeBlockDefinition* node) {

    // annotate<> shouldn't be used here, because otherwise the node will
    // have an incomplete type (in particular, it won't have its return type
    // set before other infer_type calls). This could lead to problems with
    // recursive blocks, so the node is annotated only after it is fully processed.
    auto blok_type = get_current_type_table().create<bonk::BlokType>();

    if (node->block_parameters) {
        for (auto& parameter : node->block_parameters->parameters) {
            blok_type->parameters.push_back(parameter.get());
        }
    }

    // Note: infer_block_return_type() might not infer types of
    // the block's statements, because it works in a separate
    // type table. As it assumes that this block never returns,
    // it might infer type 'never' for some statements, so these
    // types are destroyed along with the nested type table.
    blok_type->return_type = infer_block_return_type(node);

    get_current_type_table().annotate(node, blok_type);
}

std::vector<bonk::TreeNodeBonkStatement*>
bonk::TypeInferringVisitor::get_bonk_statements_in_block(TreeNodeBlockDefinition* node) {

    std::vector<TreeNodeBonkStatement*> bonk_statements;

    struct BonkVisitor : public ASTVisitor {
        std::vector<TreeNodeBonkStatement*>& bonk_statements;
        explicit BonkVisitor(std::vector<TreeNodeBonkStatement*>& bonk_statements)
            : bonk_statements(bonk_statements) {
        }

        void visit(TreeNodeBonkStatement* node) override {
            ASTVisitor::visit(node);
            bonk_statements.push_back(node);
        }
    };

    BonkVisitor bonk_visitor(bonk_statements);

    node->accept(&bonk_visitor);

    return bonk_statements;
}

std::unique_ptr<bonk::Type>
bonk::TypeInferringVisitor::infer_block_return_type(bonk::TreeNodeBlockDefinition* node) {
    if (std::find(block_stack.begin(), block_stack.end(), node) != block_stack.end()) {
        // If there is a loop in the block graph, there is no way to return
        // from these blocks, so their return type is "never"
        auto result = std::make_unique<TrivialType>();
        result->trivial_kind = TrivialTypeKind::t_never;
        return result;
    }

    // Find all bonk statements in the block

    std::vector<TreeNodeBonkStatement*> bonk_statements = get_bonk_statements_in_block(node);

    Type* return_type_annotation =
        node->return_type ? infer_type(node->return_type.get()) : nullptr;

    if (!node->body) {
        if (!return_type_annotation) {
            middle_end.linked_compiler.error().at(node->source_position)
                << "Blok should either have a return type annotation or a body";
            return std::make_unique<ErrorType>();
        }
    }

    if (node->body && bonk_statements.empty()) {
        if (return_type_annotation && return_type_annotation->is(TrivialTypeKind::t_nothing) &&
            return_type_annotation->is(TypeKind::error)) {
            middle_end.linked_compiler.error().at(node->source_position)
                << "Blok has no bonk statements, but return type is " << *return_type_annotation;
            return TypeCloner().clone(return_type_annotation);
        }
        auto type = std::make_unique<TrivialType>();
        type->trivial_kind = TrivialTypeKind::t_nothing;
        return type;
    }

    if (return_type_annotation) {
        return TypeCloner().clone(return_type_annotation);
    }

    // Infer type of each bonk statement, assuming that the block never returns
    // Because of this assumption, some inferred types might be incorrect, so they are
    // stored in a separate type table, which is destroyed after the type inference is done.
    push_type_table();

    // The block is pushed to the stack of blocks that are currently being processed.
    // Type-checker will assume that these blocks never return.
    block_stack.push_back(node);

    Type* return_type = nullptr;
    TrivialType* never_found = nullptr;

    for (auto& bonk_statement : bonk_statements) {
        auto bonk_type = infer_type(bonk_statement);

        if (!bonk_type || bonk_type->kind == TypeKind::error)
            continue;

        if (bonk_type->is(TrivialTypeKind::t_never)) {
            never_found = (TrivialType*)bonk_type;
            continue;
        }

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

    if (return_type == nullptr && never_found) {
        return_type = never_found;
    }

    // Copy the type before popping the type table, because
    // the type table will be destroyed and so may the type.
    auto result = TypeCloner().clone(return_type);

    // The inferred result can be nullptr, if the blok body
    // contains semantic errors. If that's the case, the
    // sink_types_to_parent_table is going to crash the compiler.

    if (result == nullptr) {
        result = std::make_unique<ErrorType>();
    }

    // Although some types that have been inferred in the nested
    // type table might be incorrect, it's easy to discard them
    // by filtering away the 'never'-related types. That's what the
    // sink_types_to_parent_table call does: it moves all types
    // from the nested type table to the parent type table, but
    // only if the type doesn't have 'never' inside. Thus, the type-checking
    // complexity doesn't increase to O(n^2).
    get_current_type_table().sink_types_to_parent_table();

    pop_type_table();
    return result;
}

void bonk::TypeInferringVisitor::visit(TreeNodeBonkStatement* node) {
    if (node->expression) {
        get_current_type_table().annotate(node, infer_type(node->expression.get()));
    } else {
        get_current_type_table().annotate<TrivialType>(node)->trivial_kind =
            TrivialTypeKind::t_nothing;
    }

    // TODO: Check if the bonk statement returns the same type as the blok definition

    // Find the nearest blok definition
    //    auto top_block = ...

    // If the blok definition has a return type annotation, check if the bonk statement
    // returns the same type
    //    if (top_block->return_type) {
    //        auto return_type = infer_type(top_block->return_type.get());
    //        auto bonk_type = get_current_type_table().get_type(node);
    //
    //        if (!bonk_type || bonk_type->kind == TypeKind::error)
    //            return;
    //
    //        if (!return_type || return_type->kind == TypeKind::error)
    //            return;
    //
    //        if (*return_type != *bonk_type) {
    //            middle_end.linked_compiler.error().at(node->source_position)
    //                << "bonk statement returns type " << *bonk_type
    //                << ", but blok definition returns type " << *return_type;
    //        }
    //    }
}

void bonk::TypeInferringVisitor::visit(TreeNodeVariableDefinition* node) {
    if (node->variable_type) {
        get_current_type_table().annotate(node, infer_type(node->variable_type.get()));
    } else if (node->variable_value) {
        get_current_type_table().annotate(node, infer_type(node->variable_value.get()));
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
    get_current_type_table().annotate<TrivialType>(node)->trivial_kind = TrivialTypeKind::t_nothing;
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

    get_current_type_table().annotate<ManyType>(node)->element_type =
        TypeCloner().clone(element_type);
}

void bonk::TypeInferringVisitor::visit(TreeNodeNumberConstant* node) {
    auto trivial = get_current_type_table().annotate<TrivialType>(node);

    if (node->contents.kind == NumberConstantKind::rather_double) {
        trivial->trivial_kind = bonk::TrivialTypeKind::t_flot;
    } else if (node->contents.kind == NumberConstantKind::rather_integer) {
        trivial->trivial_kind = bonk::TrivialTypeKind::t_nubr;
    } else {
        assert(!"Unknown number constant kind");
    }
}

void bonk::TypeInferringVisitor::visit(TreeNodeStringConstant* node) {
    get_current_type_table().annotate<TrivialType>(node)->trivial_kind =
        bonk::TrivialTypeKind::t_strg;
}

void bonk::TypeInferringVisitor::visit(TreeNodeBinaryOperation* node) {
    auto left_type = infer_type(node->left.get());
    auto right_type = infer_type(node->right.get());

    if (left_type->kind == TypeKind::error || right_type->kind == TypeKind::error)
        return;

    if (left_type->is(TrivialTypeKind::t_never)) {
        // There are no binary operations that can return if the left operand has type 'never'.
        // using left_type here to avoid allocating a new type.
        get_current_type_table().annotate(node, left_type);
        return;
    }

    if (right_type->is(TrivialTypeKind::t_never)) {
        if (node->operator_type == OperatorType::o_and ||
            node->operator_type == OperatorType::o_or) {
            // If operation is 'and' or 'or', then the operation can only
            // return the left operand type.
            get_current_type_table().annotate(node, left_type);
        } else {
            // Otherwise, both operands are calculated unconditionally,
            // so the operation will never return.
            // using right_type here to avoid allocating a new type.
            get_current_type_table().annotate(node, right_type);
        }
        return;
    }

    // Check if the operation is allowed between the two types
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
        get_current_type_table().annotate(node, left_type);
        break;
    case OperatorType::o_equal:
    case OperatorType::o_less:
    case OperatorType::o_greater:
    case OperatorType::o_less_equal:
    case OperatorType::o_greater_equal:
    case OperatorType::o_not_equal:
        get_current_type_table().annotate<TrivialType>(node)->trivial_kind =
            bonk::TrivialTypeKind::t_buul;
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

    get_current_type_table().annotate(node, type);
}

void bonk::TypeInferringVisitor::visit(TreeNodePrimitiveType* node) {
    get_current_type_table().annotate<TrivialType>(node)->trivial_kind = node->primitive_type;
}

void bonk::TypeInferringVisitor::visit(TreeNodeManyType* node) {
    get_current_type_table().annotate<ManyType>(node)->element_type =
        TypeCloner().clone(infer_type(node->parameter.get()));
}

void bonk::TypeInferringVisitor::visit(TreeNodeHiveAccess* node) {
    // Determine the type of the hive
    auto type = infer_type(node->hive.get());

    if (type->kind == TypeKind::external) {
        type = ((ExternalType*)type)->get_resolved();
    }

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

    get_current_type_table().annotate(node, infer_type(definition));
}

void bonk::TypeInferringVisitor::visit(TreeNodeLoopStatement* node) {
    assert(!"Cannot infer type of loop statement");
}

void bonk::TypeInferringVisitor::visit(TreeNodeHiveDefinition* node) {
    get_current_type_table().annotate<HiveType>(node)->hive_definition = node;
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
    get_current_type_table().annotate(node, function_type->return_type.get());

    // Infer type of expressions in arguments
    // and check if they are compatible with function arguments
    if (!node->arguments)
        return;

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
                << "Cannot find function parameter '" << argument->parameter_name->identifier_text
                << "' in function '" << *function_type << "'";
            continue;
        }

        auto parameter_name = argument->parameter_name.get();
        middle_end.symbol_table.symbol_definitions[parameter_name] =
            SymbolDefinition::local(definition);

        Type* valid_type = infer_type(definition);

        if (valid_type->kind == TypeKind::error)
            continue;

        if (*valid_type != *argument_type) {
            middle_end.linked_compiler.error().at(argument->parameter_value->source_position)
                << "Cannot pass '" << *argument_type << "' to parameter '"
                << argument->parameter_name->identifier_text << "' of type '" << *valid_type << "'";
        }
    }
}

void bonk::TypeInferringVisitor::visit(TreeNodeCast* node) {
    auto type = infer_type(node->target_type.get());

    if (type->kind == TypeKind::error)
        return;

    if(node->operand) {
        // TypeAnnotator might not be able to get there,
        // as it could be an identifier.
        infer_type(node->operand.get());
    }

    get_current_type_table().annotate(node, type);
}

bonk::Type* bonk::TypeInferringVisitor::infer_type(bonk::TreeNode* node) {
    // Note: when infer_type is called on a node, it is assumed
    // that the node parent has already been visited and its type
    // has been annotated

    auto& type_table = get_current_type_table();
    auto type = type_table.get_type(node);

    if (type) {
        return type;
    }

    node->accept(this);

    type = type_table.get_type(node);

    if (!type) {
        return type_table.annotate<ErrorType>(node);
    }

    return type;
}

bonk::TypeTable& bonk::TypeInferringVisitor::get_current_type_table() {
    if (type_table_stack.empty()) {
        return middle_end.type_table;
    }
    return *type_table_stack.back();
}

void bonk::TypeInferringVisitor::push_type_table() {
    TypeTable& current_type_table = get_current_type_table();
    auto new_table = std::make_unique<TypeTable>();
    new_table->parent_table = &current_type_table;
    type_table_stack.push_back(std::move(new_table));
}

void bonk::TypeInferringVisitor::pop_type_table() {
    assert(!type_table_stack.empty());
    type_table_stack.pop_back();
}

void bonk::TypeInferringVisitor::visit(bonk::TreeNodeNull* node) {
    get_current_type_table().annotate<NullType>(node);
}
