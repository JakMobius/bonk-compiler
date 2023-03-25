
#include "ast_clone_visitor.hpp"

void bonk::ASTCloneVisitor::visit(TreeNodeProgram* node) {
    auto copy = shallow_copy(node);

    for (auto& help_statement : node->help_statements) {
        auto help_statement_copy = clone(help_statement.get());
        copy->help_statements.push_back(std::move(help_statement_copy));
    }

    for (auto& statement : node->body) {
        auto statement_copy = clone(statement.get());
        copy->body.push_back(std::move(statement_copy));
    }

    result = std::move(copy);
}

void bonk::ASTCloneVisitor::visit(TreeNodeHelp* node) {
    auto copy = shallow_copy(node);

    copy->string = clone(node->string.get());

    result = std::move(copy);
}

void bonk::ASTCloneVisitor::visit(TreeNodeIdentifier* node) {
    auto copy = shallow_copy(node);

    copy->identifier_text = node->identifier_text;

    result = std::move(copy);
}

void bonk::ASTCloneVisitor::visit(TreeNodeBlockDefinition* node) {
    auto copy = shallow_copy(node);

    copy->block_name = clone(node->block_name.get());
    copy->block_parameters = clone(node->block_parameters.get());
    copy->body = clone(node->body.get());

    result = std::move(copy);
}

void bonk::ASTCloneVisitor::visit(TreeNodeVariableDefinition* node) {
    auto copy = shallow_copy(node);

    copy->variable_name = clone(node->variable_name.get());
    copy->variable_value = clone(node->variable_value.get());
    copy->variable_type = clone(node->variable_type.get());

    result = std::move(copy);
}

void bonk::ASTCloneVisitor::visit(TreeNodeParameterListDefinition* node) {
    auto copy = shallow_copy(node);

    for (auto& parameter : node->parameters) {
        auto parameter_copy = clone(parameter.get());
        copy->parameters.push_back(std::move(parameter_copy));
    }

    result = std::move(copy);
}

void bonk::ASTCloneVisitor::visit(TreeNodeParameterList* node) {
    auto copy = shallow_copy(node);

    for (auto& parameter : node->parameters) {
        auto parameter_copy = clone(parameter.get());
        copy->parameters.push_back(std::move(parameter_copy));
    }

    result = std::move(copy);
}

void bonk::ASTCloneVisitor::visit(TreeNodeParameterListItem* node) {
    auto copy = shallow_copy(node);

    copy->parameter_name = clone(node->parameter_name.get());
    copy->parameter_value = clone(node->parameter_value.get());

    result = std::move(copy);
}

void bonk::ASTCloneVisitor::visit(TreeNodeCodeBlock* node) {
    auto copy = shallow_copy(node);

    for (auto& statement : node->body) {
        auto statement_copy = clone(statement.get());
        copy->body.push_back(std::move(statement_copy));
    }

    result = std::move(copy);
}

void bonk::ASTCloneVisitor::visit(TreeNodeArrayConstant* node) {
    auto copy = shallow_copy(node);

    for (auto& element : node->elements) {
        auto element_copy = clone(element.get());
        copy->elements.push_back(std::move(element_copy));
    }

    result = std::move(copy);
}

void bonk::ASTCloneVisitor::visit(TreeNodeNumberConstant* node) {
    auto copy = shallow_copy(node);

    copy->contents = node->contents;

    result = std::move(copy);
}

void bonk::ASTCloneVisitor::visit(TreeNodeStringConstant* node) {
    auto copy = shallow_copy(node);

    copy->string_value = node->string_value;

    result = std::move(copy);
}

void bonk::ASTCloneVisitor::visit(TreeNodeBinaryOperation* node) {
    auto copy = shallow_copy(node);

    copy->left = clone(node->left.get());
    copy->right = clone(node->right.get());
    copy->operator_type = node->operator_type;

    result = std::move(copy);
}

void bonk::ASTCloneVisitor::visit(TreeNodeUnaryOperation* node) {
    auto copy = shallow_copy(node);

    copy->operand = clone(node->operand.get());
    copy->operator_type = node->operator_type;

    result = std::move(copy);
}

void bonk::ASTCloneVisitor::visit(TreeNodePrimitiveType* node) {
    auto copy = shallow_copy(node);

    copy->primitive_type = node->primitive_type;

    result = std::move(copy);
}

void bonk::ASTCloneVisitor::visit(TreeNodeManyType* node) {
    auto copy = shallow_copy(node);

    copy->parameter = clone(node->parameter.get());

    result = std::move(copy);
}

void bonk::ASTCloneVisitor::visit(TreeNodeHiveAccess* node) {
    auto copy = shallow_copy(node);

    copy->hive = clone(node->hive.get());
    copy->field = clone(node->field.get());

    result = std::move(copy);
}

void bonk::ASTCloneVisitor::visit(TreeNodeBonkStatement* node) {
    auto copy = shallow_copy(node);

    copy->expression = clone(node->expression.get());

    result = std::move(copy);
}

void bonk::ASTCloneVisitor::visit(TreeNodeBrekStatement* node) {
    auto copy = shallow_copy(node);

    result = std::move(copy);
}

void bonk::ASTCloneVisitor::visit(TreeNodeLoopStatement* node) {
    auto copy = shallow_copy(node);

    copy->body = clone(node->body.get());

    result = std::move(copy);
}

void bonk::ASTCloneVisitor::visit(TreeNodeHiveDefinition* node) {
    auto copy = shallow_copy(node);

    copy->hive_name = clone(node->hive_name.get());

    for (auto& field : node->body) {
        auto field_copy = clone(field.get());
        copy->body.push_back(std::move(field_copy));
    }

    result = std::move(copy);
}

void bonk::ASTCloneVisitor::visit(TreeNodeCall* node) {
    auto copy = shallow_copy(node);

    copy->callee = clone(node->callee.get());
    copy->arguments = clone(node->arguments.get());

    result = std::move(copy);
}
