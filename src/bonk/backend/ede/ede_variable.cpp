
#include "ede_variable.hpp"

namespace bonk::ede_backend {

Variable::Variable(TreeNodeIdentifier* the_identifier) {
    identifier = the_identifier;
}

VariableNumber::VariableNumber(TreeNodeIdentifier* identifier) : Variable(identifier) {
    type = VARIABLE_TYPE_NUMBER;
}

VariableFunction::VariableFunction(TreeNodeIdentifier* identifier, FieldList* argument_list)
    : Variable(identifier) {
    type = VARIABLE_TYPE_FUNCTION;
    this->argument_list = argument_list;
}

} // namespace bonk::ede_backend