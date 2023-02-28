
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

void TreeNodeList::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

}