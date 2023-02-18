
#pragma once

namespace bonk::ede_backend {

struct scope_stack;

}

#include <vector>
#include "../../compiler.hpp"
#include "ede_variable.hpp"

namespace bonk::ede_backend {

struct scope_stack {
    std::vector<field_list*> scopes;

    scope_stack();

    bool push_scope(field_list* scope);

    void pop_scope();

    field_list* top();

    unsigned long frame_size();

    variable* get_variable(tree_node_identifier* identifier, field_list** scope);
};

} // namespace bonk::ede_backend
