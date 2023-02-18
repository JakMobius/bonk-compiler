
#include "block_definition.hpp"

namespace bonk {

tree_node_block_definition::tree_node_block_definition() : tree_node() {
    body = nullptr;
    block_name = nullptr;

    type = TREE_NODE_TYPE_BLOCK_DEFINITION;
}

tree_node_block_definition::~tree_node_block_definition() {
    if (body)
        delete body;
    if (block_name)
        delete block_name;

    body = nullptr;
    block_name = nullptr;
}

void tree_node_block_definition::serialize(json_serializer* serializer) {
    tree_node::serialize(serializer);

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

} // namespace bonk