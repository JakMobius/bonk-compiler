
#include "ast.hpp"
#include "ast_visitor.hpp"

namespace bonk {

const char* OPERATOR_TYPE_NAMES[] = {"PLUS",
                                     "MINUS",
                                     "MULTIPLY",
                                     "DIVIDE",
                                     "ASSIGNMENT",
                                     "EQUALS",
                                     "NOT_EQUAL",
                                     "LESS_THAN",
                                     "GREATER_THAN",
                                     "LESS_OR_EQUAL_THAN",
                                     "GREATER_OR_EQUAL_THAN",
                                     "CYCLE",
                                     "CHECK",
                                     "PRINT",
                                     "BONK",
                                     "BREK",
                                     "BAMS",
                                     "AND",
                                     "OR",
                                     "REBONK",
                                     "INVALID"};

TreeNodeBlockDefinition::TreeNodeBlockDefinition() : TreeNode() {
    type = TREE_NODE_TYPE_BLOCK_DEFINITION;
}

void TreeNodeBlockDefinition::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

TreeNodeCall::TreeNodeCall() : TreeNode() {
    type = TREE_NODE_TYPE_CALL;
}

void TreeNodeCall::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

TreeNodeCallParameter::TreeNodeCallParameter()
    : TreeNode() {
    type = TREE_NODE_TYPE_CALL_PARAMETER;
}

void TreeNodeCallParameter::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

TreeNodeCheck::TreeNodeCheck() : TreeNode() {
    type = TREE_NODE_TYPE_CHECK;
}

void TreeNodeCheck::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

TreeNodeCycle::TreeNodeCycle() : TreeNode() {
    type = TREE_NODE_TYPE_CYCLE;
}

void TreeNodeCycle::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

TreeNodeIdentifier::TreeNodeIdentifier() : TreeNode() {
    type = TREE_NODE_TYPE_IDENTIFIER;
}

void TreeNodeIdentifier::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

TreeNodeList::TreeNodeList() {
    type = TREE_NODE_TYPE_LIST;
}

void TreeNodeList::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

TreeNodeNumber::TreeNodeNumber()
    : TreeNode() {
    type = TREE_NODE_TYPE_NUMBER;
}

void TreeNodeNumber::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

TreeNodeOperator::TreeNodeOperator() {
    type = TREE_NODE_TYPE_OPERATOR;
}

void TreeNodeOperator::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

TreeNodeVariableDefinition::TreeNodeVariableDefinition() {
    type = TREE_NODE_TYPE_VAR_DEFINITION;
}

void TreeNodeVariableDefinition::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

} // namespace bonk
