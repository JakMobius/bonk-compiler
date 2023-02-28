
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

TreeNodeVariableDefinition::~TreeNodeVariableDefinition() {
    delete variable_name;
    delete variable_value;

    variable_name = nullptr;
    variable_value = nullptr;
}
void TreeNodeVariableDefinition::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

} // namespace bonk