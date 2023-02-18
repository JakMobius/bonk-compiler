
#include "call_parameter.hpp"

namespace bonk {

tree_node_call_parameter::tree_node_call_parameter(tree_node_identifier* name, tree_node* value)
    : tree_node() {
    parameter_name = name;
    parameter_value = value;
    type = TREE_NODE_TYPE_CALL_PARAMETER;
}

tree_node_call_parameter::~tree_node_call_parameter() {
    delete parameter_name;
    delete parameter_value;
    parameter_name = nullptr;
    parameter_value = nullptr;
}

void tree_node_call_parameter::serialize(json_serializer* serializer) {
    tree_node::serialize(serializer);

    serializer->block_string_field("type", "call_parameter");
    if (parameter_name) {
        serializer->block_start_block("parameter_name");
        parameter_name->serialize(serializer);
        serializer->close_block();
    } else {
        serializer->block_string_field("parameter_name", nullptr);
    }

    if (parameter_value) {
        serializer->block_start_block("parameter_value");
        parameter_value->serialize(serializer);
        serializer->close_block();
    } else {
        serializer->block_string_field("parameter_value", nullptr);
    }
}
} // namespace bonk