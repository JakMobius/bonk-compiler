
#include "call_parameter.hpp"
#include "bonk/tree/ast_visitor.hpp"

namespace bonk {

TreeNodeCallParameter::TreeNodeCallParameter(TreeNodeIdentifier* name, TreeNode* value)
    : TreeNode() {
    parameter_name = name;
    parameter_value = value;
    type = TREE_NODE_TYPE_CALL_PARAMETER;
}

TreeNodeCallParameter::~TreeNodeCallParameter() {
    delete parameter_name;
    delete parameter_value;
    parameter_name = nullptr;
    parameter_value = nullptr;
}

void TreeNodeCallParameter::serialize(JsonSerializer* serializer) {
    TreeNode::serialize(serializer);

    serializer->block_string_field("type", "call_parameter");
    if (parameter_name) {
        serializer->block_start_block("parameter_name");
        parameter_name->serialize(serializer);
        serializer->close_block();
    } else {
        serializer->block_string_field("parameter_name", nullptr);
    }

    if (parameter_value) {
        serializer->block_start_block("parameter_value");
        parameter_value->serialize(serializer);
        serializer->close_block();
    } else {
        serializer->block_string_field("parameter_value", nullptr);
    }
}
void TreeNodeCallParameter::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}
} // namespace bonk