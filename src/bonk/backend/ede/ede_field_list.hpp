
#pragma once

namespace bonk::ede_backend {

struct field_list;

}

#include <vector>
#include "ede_variable.hpp"

namespace bonk::ede_backend {

struct field_list {
    std::vector<variable*> variables;
    unsigned long byte_offset;
    unsigned long frame_size;

    field_list();

    ~field_list();

    variable* get_variable(tree_node_identifier* identifier);

    bool add_variable(variable* variable);
};

} // namespace bonk::ede_backend
