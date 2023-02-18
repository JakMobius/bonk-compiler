
#include "cycle.hpp"

namespace bonk {

tree_node_cycle::tree_node_cycle() : tree_node() {
    type = TREE_NODE_TYPE_CYCLE;
}

void tree_node_cycle::serialize(json_serializer* serializer) {
    tree_node::serialize(serializer);
    serializer->block_string_field("type", "cycle");
    if (body) {
        serializer->block_start_block("body");
        body->serialize(serializer);
        serializer->close_block();
    } else {
        serializer->block_string_field("body", nullptr);
    }
}

} // namespace bonk