
#include "list.hpp"
#include "bonk/tree/ast_visitor.hpp"

namespace bonk {

TreeNodeList::TreeNodeList() {
    type = TREE_NODE_TYPE_LIST;
}

TreeNodeList::~TreeNodeList() {
    for (auto& i : list) {
        delete i;
    }
}

void TreeNodeList::serialize(JsonSerializer* serializer) {
    TreeNode::serialize(serializer);

    serializer->block_string_field("type", "list");
    serializer->block_start_array("contents");

    for (auto element : list) {
        if (element) {
            serializer->array_add_block();
            element->serialize(serializer);
            serializer->close_block();
        } else
            serializer->array_add_string(nullptr);
    }

    serializer->close_array();
}

void TreeNodeList::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

}