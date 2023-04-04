
#include "type_annotator.hpp"
#include "../../compiler.hpp"
#include "../middleend.hpp"
#include "type_inferring.hpp"

bonk::TypeAnnotator::TypeAnnotator(bonk::MiddleEnd& middle_end) : middle_end(middle_end) {
}

bonk::Type* bonk::TypeAnnotator::infer_type(bonk::TreeNode* node) {
    TypeInferringVisitor visitor{middle_end};
    return visitor.infer_type(node);
}

void bonk::TypeAnnotator::visit(bonk::TreeNodeHiveDefinition* node) {
    infer_type(node);
    ASTVisitor::visit(node);
}

void bonk::TypeAnnotator::visit(bonk::TreeNodeVariableDefinition* node) {
    infer_type(node);
    ASTVisitor::visit(node);
}

void bonk::TypeAnnotator::visit(bonk::TreeNodeBlockDefinition* node) {
    infer_type(node);
    ASTVisitor::visit(node);
}

void bonk::TypeAnnotator::visit(bonk::TreeNodeHiveAccess* node) {
    infer_type(node);
    ASTVisitor::visit(node);
}

void bonk::TypeAnnotator::visit(bonk::TreeNodeCall* node) {
    infer_type(node);
    Type* type = infer_type(node->callee.get());

    if (!type || type->kind == TypeKind::error)
        return;
    ASTVisitor::visit(node);
}

void bonk::TypeAnnotator::visit(bonk::TreeNodeCast* node) {
    infer_type(node);
    ASTVisitor::visit(node);
}

void bonk::TypeAnnotator::visit(bonk::TreeNodeParameterListItem* node) {
    if (node->parameter_value)
        node->parameter_value->accept(this);

    // Don't accept 'parameter_name', because TypeInferringVisitor will not be able
    // to infer type of parameter name.
}

void bonk::TypeAnnotator::visit(TreeNodeArrayConstant* node) {
    ASTVisitor::visit(node);
    infer_type(node);
}

void bonk::TypeAnnotator::visit(TreeNodeNumberConstant* node) {
    ASTVisitor::visit(node);
    infer_type(node);
}

void bonk::TypeAnnotator::visit(TreeNodeStringConstant* node) {
    ASTVisitor::visit(node);
    infer_type(node);
}

void bonk::TypeAnnotator::visit(TreeNodeBinaryOperation* node) {
    ASTVisitor::visit(node);
    infer_type(node);
}

void bonk::TypeAnnotator::visit(TreeNodeUnaryOperation* node) {
    ASTVisitor::visit(node);
    infer_type(node);
}

void bonk::TypeAnnotator::visit(TreeNodePrimitiveType* node) {
    ASTVisitor::visit(node);
    infer_type(node);
}

void bonk::TypeAnnotator::visit(TreeNodeManyType* node) {
    ASTVisitor::visit(node);
    infer_type(node);
}

void bonk::TypeAnnotator::visit(TreeNodeBonkStatement* node) {
    ASTVisitor::visit(node);
    infer_type(node);
}

void bonk::TypeAnnotator::annotate_ast(bonk::AST& ast) {
    ast.root->accept(this);
}

void bonk::TypeAnnotator::visit(bonk::TreeNodeNull* node) {
    infer_type(node);
}

bonk::HiveFieldNameResolver::HiveFieldNameResolver(TreeNodeHiveDefinition* hive_definition)
    : hive_definition(hive_definition) {
}

bonk::TreeNode* bonk::HiveFieldNameResolver::get_name_definition(std::string_view name) {

    for (auto& field : hive_definition->body) {
        switch (field->type) {
        case TreeNodeType::n_variable_definition: {
            auto variable_definition = (TreeNodeVariableDefinition*)field.get();
            if (variable_definition->variable_name->identifier_text == name) {
                return variable_definition;
            }
            break;
        }
        case TreeNodeType::n_block_definition: {
            auto block_definition = (TreeNodeBlockDefinition*)field.get();
            if (block_definition->block_name->identifier_text == name) {
                return block_definition;
            }
            break;
        }
        default:
            break;
        }
    }

    return nullptr;
}

bonk::FunctionParameterNameResolver::FunctionParameterNameResolver(bonk::BlokType* called_function)
    : called_function(called_function) {
}

bonk::TreeNode* bonk::FunctionParameterNameResolver::get_name_definition(std::string_view name) {

    for (auto& parameter : called_function->parameters) {
        if (parameter->variable_name->identifier_text == name) {
            return parameter;
        }
    }

    return nullptr;
}
