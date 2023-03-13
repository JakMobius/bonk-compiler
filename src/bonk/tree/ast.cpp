
#include "ast.hpp"
#include "ast_visitor.hpp"

namespace bonk {

const char* BONK_PRIMITIVE_TYPE_NAMES[] = {
    "unset",
    "buul",
    "shrt",
    "nubr",
    "long",
    "flot",
    "dabl",
    "strg"
};

void TreeNodeProgram::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}
void TreeNodeHelp::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}
void TreeNodeIdentifier::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}
void TreeNodeBlockDefinition::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}
void TreeNodeHiveDefinition::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}
void TreeNodeVariableDefinition::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}
void TreeNodeParameterListDefinition::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}
void TreeNodeParameterList::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}
void TreeNodeParameterListItem::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}
void TreeNodeCodeBlock::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}
void TreeNodeBonkStatement::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}
void TreeNodeBrekStatement::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}
void TreeNodeArrayConstant::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}
void TreeNodeNumberConstant::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}
void TreeNodeStringConstant::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}
void TreeNodeHiveAccess::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}
void TreeNodeLoopStatement::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}
void TreeNodeManyType::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}
void TreeNodePrimitiveType::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}
void TreeNodeBinaryOperation::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}
void TreeNodeUnaryOperation::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}
void TreeNodeCall::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}
} // namespace bonk
