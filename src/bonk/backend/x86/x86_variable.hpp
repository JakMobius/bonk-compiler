#pragma once

namespace bonk::x86_backend {

struct variable;
struct variable_function;

enum variable_type { VARIABLE_TYPE_NUMBER, VARIABLE_TYPE_FUNCTION };

} // namespace bonk::x86_backend

#include "../../tree/nodes/identifier.hpp"
#include "x86_abstract_register.hpp"
#include "x86_field_list.hpp"

namespace bonk::x86_backend {

struct variable {
    bool is_contextual;
    variable_type type;
    tree_node_identifier* identifier;
    abstract_register storage;

    variable(tree_node_identifier* identifier);
};

struct variable_number : variable {
    variable_number(tree_node_variable_definition* definition);
};

struct variable_function : variable {
    field_list* argument_list;

    variable_function(tree_node_identifier* the_identifier, field_list* the_argument_list);
};

} // namespace bonk::x86_backend
