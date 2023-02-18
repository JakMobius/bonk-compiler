
#include "x86_variable.hpp"

namespace bonk::x86_backend {

variable::variable(tree_node_identifier* the_identifier) {
    identifier = the_identifier;
}

variable_number::variable_number(tree_node_variable_definition* definition)
    : variable(definition->variable_name) {
    this->is_contextual = definition->is_contextual;
    type = VARIABLE_TYPE_NUMBER;
}

variable_function::variable_function(tree_node_identifier* identifier,
                                     field_list* the_argument_list)
    : variable(identifier) {
    argument_list = the_argument_list;
    type = VARIABLE_TYPE_FUNCTION;
}

} // namespace bonk::x86_backend