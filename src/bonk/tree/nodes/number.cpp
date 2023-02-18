
#include "number.hpp"

namespace bonk {

tree_node_number::tree_node_number(long double the_float_value, long long the_integer_value)
    : tree_node() {
    float_value = the_float_value;
    integer_value = the_integer_value;
    type = TREE_NODE_TYPE_NUMBER;
}

void tree_node_number::serialize(json_serializer* serializer) {
    tree_node::serialize(serializer);

    serializer->block_string_field("type", "root_list");
    serializer->block_number_field("float_value", float_value);
    serializer->block_number_field("integer_value", integer_value);
}

} // namespace bonk