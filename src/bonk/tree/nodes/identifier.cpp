
#include "identifier.hpp"

namespace bonk {

tree_node_identifier::tree_node_identifier(const std::string& name) : tree_node() {
    variable_name = name;
    type = TREE_NODE_TYPE_IDENTIFIER;
}

bool tree_node_identifier::contents_equal(tree_node_identifier* other) {
    return variable_name == other->variable_name;
}

void tree_node_identifier::print(FILE* file) {
    fprintf(file, "%s", variable_name.c_str());
}

void tree_node_identifier::serialize(json_serializer* serializer) {
    tree_node::serialize(serializer);

    serializer->block_string_field("type", "identifier");
    serializer->block_string_field("identifier", variable_name.c_str());
}

} // namespace bonk