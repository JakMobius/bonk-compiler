
#pragma once

namespace bonk::x86_backend {

struct field_list;

}

#include <vector>
#include "../../tree/nodes/nodes.hpp"
#include "x86_backend_context.hpp"
#include "x86_variable.hpp"

namespace bonk::x86_backend {

struct field_list {
    int arguments;
    std::vector<variable*> variables;
    backend_context_state* context_state;
    register_descriptor_list* descriptors;

    field_list(register_descriptor_list* descriptor_list, backend_context_state* state);

    ~field_list();

    variable* get_variable(tree_node_identifier* identifier);

    bool add_variable(variable* variable);
};

} // namespace bonk::x86_backend
