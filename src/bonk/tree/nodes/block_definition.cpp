
#include "block_definition.hpp"
#include "bonk/tree/ast_visitor.hpp"

namespace bonk {

TreeNodeBlockDefinition::TreeNodeBlockDefinition() : TreeNode() {
    body = nullptr;
    block_name = nullptr;

    type = TREE_NODE_TYPE_BLOCK_DEFINITION;
}

TreeNodeBlockDefinition::~TreeNodeBlockDefinition() {

    delete body;

    delete block_name;

    body = nullptr;
    block_name = nullptr;
}

void TreeNodeBlockDefinition::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

} // namespace bonk