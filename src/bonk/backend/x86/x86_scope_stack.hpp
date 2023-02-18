
#pragma once

namespace bonk::x86_backend {

struct scope_stack;

}

#include <vector>
#include "../../compiler.hpp"
#include "x86_variable.hpp"

namespace bonk::x86_backend {

struct scope_stack {
    std::vector<field_list*> scopes;

    scope_stack();

    void push_scope(field_list* scope);

    void pop_scope();

    field_list* top();

    variable* get_variable(tree_node_identifier* identifier, field_list** scope);
};

} // namespace bonk::x86_backend
