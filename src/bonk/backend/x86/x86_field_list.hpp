
#pragma once

namespace bonk::x86_backend {

struct FieldList;

}

#include <vector>
#include "bonk/tree/ast.hpp"
#include "x86_backend_context.hpp"
#include "x86_variable.hpp"

#if 0

namespace bonk::x86_backend {

struct FieldList {
    int arguments;
    std::vector<Variable*> variables{};
    BackendContextState* context_state;
    RegisterDescriptorList* descriptors;

    FieldList(RegisterDescriptorList* descriptor_list, BackendContextState* state);

    ~FieldList();

    Variable* get_variable(TreeNodeIdentifier* identifier);

    void add_variable(Variable* variable);
};

} // namespace bonk::x86_backend

#endif
