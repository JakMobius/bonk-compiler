#pragma once

namespace bonk {

struct TreeNode;
struct TreeNodeMethods;
struct ASTVisitor;

enum TreeNodeType {
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

} // namespace bonk

#include <cstdio>
#include "../../../utils/json_serializer.hpp"
#include "../../parsing/parser_position.hpp"

namespace bonk {

struct TreeNode {
    TreeNodeType type = TREE_NODE_TYPE_INVALID;
    ParserPosition* source_position = nullptr;

    TreeNode() = default;

    virtual ~TreeNode() = default;

    virtual void accept(ASTVisitor* visitor) {}

    virtual void serialize(JsonSerializer* file);
};

} // namespace bonk
