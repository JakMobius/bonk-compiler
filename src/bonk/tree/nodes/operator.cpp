
#include "operator.hpp"

namespace bonk {

const char* OPERATOR_TYPE_NAMES[] = {"PLUS",
                                     "MINUS",
                                     "MULTIPLY",
                                     "DIVIDE",
                                     "ASSIGNMENT",
                                     "EQUALS",
                                     "NOT_EQUAL",
                                     "LESS_THAN",
                                     "GREATER_THAN",
                                     "LESS_OR_EQUAL_THAN",
                                     "GREATER_OR_EQUAL_THAN",
                                     "CYCLE",
                                     "CHECK",
                                     "PRINT",
                                     "BONK",
                                     "BREK",
                                     "BAMS",
                                     "AND",
                                     "OR",
                                     "REBONK",
                                     "INVALID"};

tree_node_operator::tree_node_operator(operator_type oper) {
    oper_type = oper;
    type = TREE_NODE_TYPE_OPERATOR;
}

void tree_node_operator::serialize(json_serializer* serializer) {
    tree_node::serialize(serializer);

    serializer->block_string_field("type", "operator");
    serializer->block_string_field("operator_type", OPERATOR_TYPE_NAMES[oper_type]);

    if (left) {
        serializer->block_start_block("left");
        left->serialize(serializer);
        serializer->close_block();
    } else {
        serializer->block_string_field("left", nullptr);
    }
    if (right) {
        serializer->block_start_block("right");
        right->serialize(serializer);
        serializer->close_block();
    } else {
        serializer->block_string_field("right", nullptr);
    }
}

} // namespace bonk