
#include "block_definition.hpp"
#include "bonk/tree/ast_visitor.hpp"

namespace bonk {

TreeNodeBlockDefinition::TreeNodeBlockDefinition() : TreeNode() {
    body = nullptr;
    block_name = nullptr;

    type = TREE_NODE_TYPE_BLOCK_DEFINITION;
}

TreeNodeBlockDefinition::~TreeNodeBlockDefinition() {

    delete body;

    delete block_name;

    body = nullptr;
    block_name = nullptr;
}

void TreeNodeBlockDefinition::serialize(JsonSerializer* serializer) {
    TreeNode::serialize(serializer);

    serializer->block_string_field("type", "block_definition");
    serializer->block_string_field("block_name",
                                   block_name ? block_name->variable_name.c_str() : nullptr);
    if (body) {
        serializer->block_start_block("body");
        body->serialize(serializer);
        serializer->close_block();
    } else {
        serializer->block_string_field("body", nullptr);
    }
}
void TreeNodeBlockDefinition::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

} // namespace bonk