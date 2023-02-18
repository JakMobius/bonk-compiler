
#include "var_definition.hpp"

namespace bonk {

tree_node_variable_definition::tree_node_variable_definition(bool contextual,
                                                             tree_node_identifier* identifier) {
    variable_name = identifier;
    is_contextual = contextual;
    variable_value = nullptr;
    type = TREE_NODE_TYPE_VAR_DEFINITION;
}

void tree_node_variable_definition::serialize(json_serializer* serializer) {
    tree_node::serialize(serializer);

    serializer->block_string_field("type", "var_definition");
    if (variable_name) {
        serializer->block_start_block("variable_name");
        variable_name->serialize(serializer);
        serializer->close_block();
    } else {
        serializer->block_string_field("variable_name", nullptr);
    }

    if (variable_value) {
        serializer->block_start_block("variable_value");
        variable_value->serialize(serializer);
        serializer->close_block();
    } else {
        serializer->block_string_field("variable_value", nullptr);
    }
}

tree_node_variable_definition::~tree_node_variable_definition() {
    delete variable_name;
    delete variable_value;

    variable_name = nullptr;
    variable_value = nullptr;
}

} // namespace bonk