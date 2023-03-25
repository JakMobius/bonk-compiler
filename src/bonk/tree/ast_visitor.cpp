
#include "ast_visitor.hpp"

namespace bonk {

void ASTVisitor::visit(TreeNodeProgram* node) {
    for (auto& element : node->help_statements) {
        if (element) {
            element->accept(this);
        }
    }
    for (auto& element : node->body) {
        if (element) {
            element->accept(this);
        }
    }
}
void ASTVisitor::visit(TreeNodeHelp* node) {
    if(node->identifier) node->identifier->accept(this);
}
void ASTVisitor::visit(TreeNodeIdentifier* node) {
}
void ASTVisitor::visit(TreeNodeBlockDefinition* node) {
    if(node->block_name) node->block_name->accept(this);
    if(node->block_parameters) node->block_parameters->accept(this);
    if(node->return_type) node->return_type->accept(this);
    if(node->body) node->body->accept(this);
}
void ASTVisitor::visit(TreeNodeVariableDefinition* node) {
    if(node->variable_name) node->variable_name->accept(this);
    if(node->variable_type) node->variable_type->accept(this);
    if(node->variable_value) node->variable_value->accept(this);
}
void ASTVisitor::visit(TreeNodeParameterListDefinition* node) {
    for (auto& element : node->parameters) {
        if (element) {
            element->accept(this);
        }
    }
}
void ASTVisitor::visit(TreeNodeParameterList* node) {
    for (auto& element : node->parameters) {
        if (element) {
            element->accept(this);
        }
    }
}
void ASTVisitor::visit(TreeNodeParameterListItem* node) {
    if(node->parameter_name) node->parameter_name->accept(this);
    if(node->parameter_value) node->parameter_value->accept(this);
}
void ASTVisitor::visit(TreeNodeCodeBlock* node) {
    for (auto& element : node->body) {
        if (element) {
            element->accept(this);
        }
    }
}
void ASTVisitor::visit(TreeNodeArrayConstant* node) {
    for (auto& element : node->elements) {
        if (element) {
            element->accept(this);
        }
    }
}
void ASTVisitor::visit(TreeNodeNumberConstant* node) {
}
void ASTVisitor::visit(TreeNodeStringConstant* node) {
}
void ASTVisitor::visit(TreeNodeBinaryOperation* node) {
    if(node->left) node->left->accept(this);
    if(node->right) node->right->accept(this);
}
void ASTVisitor::visit(TreeNodeUnaryOperation* node) {
    if(node->operand) node->operand->accept(this);
}
void ASTVisitor::visit(TreeNodePrimitiveType* node) {
}
void ASTVisitor::visit(TreeNodeManyType* node) {
    if(node->parameter) node->parameter->accept(this);
}
void ASTVisitor::visit(TreeNodeHiveAccess* node) {
    if(node->hive) node->hive->accept(this);
    if(node->field) node->field->accept(this);
}
void ASTVisitor::visit(TreeNodeBonkStatement* node) {
    if(node->expression) node->expression->accept(this);
}
void ASTVisitor::visit(TreeNodeBrekStatement* node) {}
void ASTVisitor::visit(TreeNodeLoopStatement* node) {
    if(node->loop_parameters) node->loop_parameters->accept(this);
    if(node->body) node->body->accept(this);
}
void ASTVisitor::visit(TreeNodeHiveDefinition* node) {
    if(node->hive_name) node->hive_name->accept(this);

    for (auto& element : node->body) {
        if (element) {
            element->accept(this);
        }
    }
}
void ASTVisitor::visit(TreeNodeCall* node) {
    if(node->callee) node->callee->accept(this);
    if(node->arguments) node->arguments->accept(this);
}

} // namespace bonk