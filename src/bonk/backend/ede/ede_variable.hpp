#pragma once

namespace bonk::ede_backend {

struct variable;
struct variable_function;

enum variable_type { VARIABLE_TYPE_NUMBER, VARIABLE_TYPE_FUNCTION };

} // namespace bonk::ede_backend

#include "../../tree/nodes/identifier.hpp"
#include "ede_field_list.hpp"

namespace bonk::ede_backend {

struct variable {
    variable_type type;
    tree_node_identifier* identifier;
    unsigned long byte_offset;

    variable(tree_node_identifier* identifier);
};

struct variable_number : variable {
    variable_number(tree_node_identifier* identifier);
};

struct variable_function : variable {
    field_list* argument_list;

    variable_function(tree_node_identifier* identifier, field_list* the_argument_list);
};

} // namespace bonk::ede_backend
