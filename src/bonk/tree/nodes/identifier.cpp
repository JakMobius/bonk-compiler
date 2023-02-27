
#include "identifier.hpp"

namespace bonk {

TreeNodeIdentifier::TreeNodeIdentifier(const std::string& name) : TreeNode() {
    variable_name = name;
    type = TREE_NODE_TYPE_IDENTIFIER;
}

bool TreeNodeIdentifier::contents_equal(TreeNodeIdentifier* other) {
    return variable_name == other->variable_name;
}

void TreeNodeIdentifier::print(FILE* file) const {
    fprintf(file, "%s", variable_name.c_str());
}

void TreeNodeIdentifier::serialize(JsonSerializer* serializer) {
    TreeNode::serialize(serializer);

    serializer->block_string_field("type", "identifier");
    serializer->block_string_field("identifier", variable_name.c_str());
}

} // namespace bonk