
#include "cycle.hpp"
#include "bonk/tree/ast_visitor.hpp"

namespace bonk {

TreeNodeCycle::TreeNodeCycle() : TreeNode() {
    type = TREE_NODE_TYPE_CYCLE;
}

void TreeNodeCycle::serialize(JsonSerializer* serializer) {
    TreeNode::serialize(serializer);
    serializer->block_string_field("type", "cycle");
    if (body) {
        serializer->block_start_block("body");
        body->serialize(serializer);
        serializer->close_block();
    } else {
        serializer->block_string_field("body", nullptr);
    }
}
void TreeNodeCycle::accept(ASTVisitor* visitor) {
    visitor->visit(this);
    body->accept(visitor);
    visitor->leave(this);
}

} // namespace bonk