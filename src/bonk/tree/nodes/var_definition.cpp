
#include "var_definition.hpp"
#include "bonk/tree/ast_visitor.hpp"

namespace bonk {

TreeNodeVariableDefinition::TreeNodeVariableDefinition(bool contextual,
                                                             TreeNodeIdentifier* identifier) {
    variable_name = identifier;
    is_contextual = contextual;
    variable_value = nullptr;
    type = TREE_NODE_TYPE_VAR_DEFINITION;
}

void TreeNodeVariableDefinition::serialize(JsonSerializer* serializer) {
    TreeNode::serialize(serializer);

    serializer->block_string_field("type", "var_definition");
    if (variable_name) {
        serializer->block_start_block("variable_name");
        variable_name->serialize(serializer);
        serializer->close_block();
    } else {
        serializer->block_string_field("variable_name", nullptr);
    }

    if (variable_value) {
        serializer->block_start_block("variable_value");
        variable_value->serialize(serializer);
        serializer->close_block();
    } else {
        serializer->block_string_field("variable_value", nullptr);
    }
}

TreeNodeVariableDefinition::~TreeNodeVariableDefinition() {
    delete variable_name;
    delete variable_value;

    variable_name = nullptr;
    variable_value = nullptr;
}
void TreeNodeVariableDefinition::accept(ASTVisitor* visitor) {
    visitor->visit(this);
    variable_name->accept(visitor);
    variable_value->accept(visitor);
    visitor->leave(this);
}

} // namespace bonk