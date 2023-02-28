
#include "identifier.hpp"
#include "bonk/tree/ast_visitor.hpp"

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

void TreeNodeIdentifier::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

} // namespace bonk