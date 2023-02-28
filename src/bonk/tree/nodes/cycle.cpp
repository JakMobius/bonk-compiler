
#include "cycle.hpp"
#include "bonk/tree/ast_visitor.hpp"

namespace bonk {

TreeNodeCycle::TreeNodeCycle() : TreeNode() {
    type = TREE_NODE_TYPE_CYCLE;
}

void TreeNodeCycle::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

} // namespace bonk