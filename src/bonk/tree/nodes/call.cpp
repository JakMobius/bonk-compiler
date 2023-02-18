
#include "call.hpp"

namespace bonk {

tree_node_call::tree_node_call(tree_node_identifier* function,
                               tree_node_list<tree_node_call_parameter*>* parameters)
    : tree_node() {
    call_function = function;
    call_parameters = parameters;
    type = TREE_NODE_TYPE_CALL;
}

tree_node_call::~tree_node_call() {
    delete call_function;
    delete call_parameters;
    call_function = nullptr;
    call_parameters = nullptr;
}

void tree_node_call::serialize(json_serializer* serializer) {
    tree_node::serialize(serializer);
    serializer->block_string_field("type", "call");

    if (call_function) {
        serializer->block_start_block("call_function");
        call_function->serialize(serializer);
        serializer->close_block();
    } else {
        serializer->block_string_field("call_function", nullptr);
    }

    if (call_parameters) {
        serializer->block_start_block("call_parameters");
        call_parameters->serialize(serializer);
        serializer->close_block();
    } else {
        serializer->block_string_field("call_parameters", nullptr);
    }
}
} // namespace bonk