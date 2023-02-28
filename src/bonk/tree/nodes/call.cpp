
#include "call.hpp"
#include "bonk/tree/ast_visitor.hpp"

namespace bonk {

TreeNodeCall::TreeNodeCall(TreeNodeIdentifier* function, TreeNodeList* parameters) : TreeNode() {
    call_function = function;
    call_parameters = parameters;
    type = TREE_NODE_TYPE_CALL;
}

TreeNodeCall::~TreeNodeCall() {
    delete call_function;
    delete call_parameters;
    call_function = nullptr;
    call_parameters = nullptr;
}

void TreeNodeCall::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}
} // namespace bonk