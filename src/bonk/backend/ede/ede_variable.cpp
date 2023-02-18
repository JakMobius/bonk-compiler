
#include "ede_variable.hpp"

namespace bonk::ede_backend {

variable::variable(tree_node_identifier* the_identifier) {
    identifier = the_identifier;
}

variable_number::variable_number(tree_node_identifier* identifier) : variable(identifier) {
    type = VARIABLE_TYPE_NUMBER;
}

variable_function::variable_function(tree_node_identifier* identifier, field_list* argument_list)
    : variable(identifier) {
    type = VARIABLE_TYPE_FUNCTION;
    this->argument_list = argument_list;
}

} // namespace bonk::ede_backend