
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

void TreeNodeCall::serialize(JsonSerializer* serializer) {
    TreeNode::serialize(serializer);
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

void TreeNodeCall::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}
} // namespace bonk