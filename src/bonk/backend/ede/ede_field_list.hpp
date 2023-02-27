
#pragma once

namespace bonk::ede_backend {

struct FieldList;

}

#include <vector>
#include "ede_variable.hpp"

namespace bonk::ede_backend {

struct FieldList {
    std::vector<Variable*> variables;
    unsigned long byte_offset;
    unsigned long frame_size;

    FieldList();

    ~FieldList();

    Variable* get_variable(TreeNodeIdentifier* identifier);

    bool add_variable(Variable* variable);
};

} // namespace bonk::ede_backend
