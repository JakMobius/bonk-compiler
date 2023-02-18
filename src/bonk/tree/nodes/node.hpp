#pragma once

#include <cstdio>
#include "../../../utils/json_serializer.hpp"
#include "../../parsing/parser_position.hpp"

namespace bonk {

struct tree_node;
struct tree_node_methods;

} // namespace bonk

namespace bonk {

enum tree_node_type {
    TREE_NODE_TYPE_NUMBER,
    TREE_NODE_TYPE_IDENTIFIER,
    TREE_NODE_TYPE_OPERATOR,
    TREE_NODE_TYPE_LIST,
    TREE_NODE_TYPE_BLOCK_DEFINITION,
    TREE_NODE_TYPE_VAR_DEFINITION,
    TREE_NODE_TYPE_CALL,
    TREE_NODE_TYPE_CALL_PARAMETER,
    TREE_NODE_TYPE_CHECK,
    TREE_NODE_TYPE_CYCLE,
    TREE_NODE_TYPE_INVALID
};

struct tree_node {
    tree_node_type type = TREE_NODE_TYPE_INVALID;
    parser_position* source_position = nullptr;

    tree_node() = default;

    virtual ~tree_node() = default;

    virtual void serialize(json_serializer* file);
};

} // namespace bonk
