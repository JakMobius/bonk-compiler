
#include "check.hpp"
#include "bonk/tree/ast_visitor.hpp"

namespace bonk {

TreeNodeCheck::TreeNodeCheck() : TreeNode() {
    condition = nullptr;
    check_body = nullptr;
    or_body = nullptr;

    type = TREE_NODE_TYPE_CHECK;
}

TreeNodeCheck::~TreeNodeCheck() {
    delete condition;
    delete check_body;
    delete or_body;

    condition = nullptr;
    check_body = nullptr;
    or_body = nullptr;
}

void TreeNodeCheck::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

} // namespace bonk