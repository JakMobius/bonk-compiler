
#include "check.hpp"

namespace bonk {

tree_node_check::tree_node_check() : tree_node() {
    condition = nullptr;
    check_body = nullptr;
    or_body = nullptr;

    type = TREE_NODE_TYPE_CHECK;
}

tree_node_check::~tree_node_check() {
    delete condition;
    delete check_body;
    delete or_body;

    condition = nullptr;
    check_body = nullptr;
    or_body = nullptr;
}

void tree_node_check::serialize(json_serializer* serializer) {
    tree_node::serialize(serializer);

    serializer->block_string_field("type", "check");

    if (check_body) {
        serializer->block_start_block("check_body");
        check_body->serialize(serializer);
        serializer->close_block();
    } else {
        serializer->block_string_field("parameter_value", nullptr);
    }

    if (or_body) {
        serializer->block_start_block("check_body");
        or_body->serialize(serializer);
        serializer->close_block();
    } else {
        serializer->block_string_field("or_body", nullptr);
    }

    if (condition) {
        serializer->block_start_block("condition");
        condition->serialize(serializer);
        serializer->close_block();
    } else {
        serializer->block_string_field("condition", nullptr);
    }
}

} // namespace bonk