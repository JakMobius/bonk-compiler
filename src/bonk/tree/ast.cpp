
#include "ast.hpp"
#include "ast_visitor.hpp"

namespace bonk {

TreeNodeBlockDefinition::TreeNodeBlockDefinition() : TreeNode() {
    type = TreeNodeType::n_block_definition;
}

void TreeNodeBlockDefinition::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

TreeNodeCall::TreeNodeCall() : TreeNode() {
    type = TreeNodeType::n_call;
}

void TreeNodeCall::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

TreeNodeCallParameter::TreeNodeCallParameter()
    : TreeNode() {
    type = TreeNodeType::n_call_parameter;
}

void TreeNodeCallParameter::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

TreeNodeCheck::TreeNodeCheck() : TreeNode() {
    type = TreeNodeType::n_check;
}

void TreeNodeCheck::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

TreeNodeCycle::TreeNodeCycle() : TreeNode() {
    type = TreeNodeType::n_cycle;
}

void TreeNodeCycle::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

TreeNodeIdentifier::TreeNodeIdentifier() : TreeNode() {
    type = TreeNodeType::n_identifier;
}

void TreeNodeIdentifier::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

TreeNodeList::TreeNodeList() {
    type = TreeNodeType::n_list;
}

void TreeNodeList::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

TreeNodeNumber::TreeNodeNumber()
    : TreeNode() {
    type = TreeNodeType::n_number;
}

void TreeNodeNumber::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

TreeNodeOperator::TreeNodeOperator() {
    type = TreeNodeType::n_operator;
}

void TreeNodeOperator::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

TreeNodeVariableDefinition::TreeNodeVariableDefinition() {
    type = TreeNodeType::n_var_definition;
}

void TreeNodeVariableDefinition::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

} // namespace bonk
