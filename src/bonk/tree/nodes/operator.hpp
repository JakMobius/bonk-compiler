#pragma once
namespace bonk {

struct TreeNodeOperator;

enum OperatorType {
    BONK_OPERATOR_PLUS,
    BONK_OPERATOR_MINUS,
    BONK_OPERATOR_MULTIPLY,
    BONK_OPERATOR_DIVIDE,
    BONK_OPERATOR_ASSIGNMENT,
    BONK_OPERATOR_EQUALS,
    BONK_OPERATOR_NOT_EQUAL,
    BONK_OPERATOR_LESS_THAN,
    BONK_OPERATOR_GREATER_THAN,
    BONK_OPERATOR_LESS_OR_EQUAL_THAN,
    BONK_OPERATOR_GREATER_OR_EQUAL_THAN,
    BONK_OPERATOR_CYCLE,
    BONK_OPERATOR_CHECK,
    BONK_OPERATOR_PRINT,
    BONK_OPERATOR_BONK,
    BONK_OPERATOR_BREK,
    BONK_OPERATOR_BAMS,
    BONK_OPERATOR_AND,
    BONK_OPERATOR_OR,
    BONK_OPERATOR_REBONK,
    BONK_OPERATOR_INVALID
};

extern const char* OPERATOR_TYPE_NAMES[];

} // namespace bonk

#include "node.hpp"
#include "utils/json_serializer.hpp"

namespace bonk {

struct TreeNodeOperator : TreeNode {
    OperatorType oper_type = BONK_OPERATOR_INVALID;
    TreeNode* left = nullptr;
    TreeNode* right = nullptr;

    explicit TreeNodeOperator(OperatorType oper);

    void serialize(JsonSerializer* file) override;
};

} // namespace bonk
