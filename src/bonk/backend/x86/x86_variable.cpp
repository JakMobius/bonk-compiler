
#include "x86_variable.hpp"

namespace bonk::x86_backend {

Variable::Variable(TreeNodeIdentifier* the_identifier) {
    identifier = the_identifier;
}

VariableNumber::VariableNumber(TreeNodeVariableDefinition* definition)
    : Variable(definition->variable_name) {
    this->is_contextual = definition->is_contextual;
    type = VARIABLE_TYPE_NUMBER;
}

VariableFunction::VariableFunction(TreeNodeIdentifier* identifier, FieldList* the_argument_list)
    : Variable(identifier) {
    argument_list = the_argument_list;
    type = VARIABLE_TYPE_FUNCTION;
}

} // namespace bonk::x86_backend