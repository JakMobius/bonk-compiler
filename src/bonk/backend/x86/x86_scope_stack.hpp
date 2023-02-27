
#pragma once

namespace bonk::x86_backend {

struct ScopeStack;

}

#include <vector>
#include "../../compiler.hpp"
#include "x86_variable.hpp"

namespace bonk::x86_backend {

struct ScopeStack {
    std::vector<FieldList*> scopes{};

    ScopeStack();

    void push_scope(FieldList* scope);

    void pop_scope();

    FieldList* top();

    Variable* get_variable(TreeNodeIdentifier* identifier, FieldList** scope);
};

} // namespace bonk::x86_backend
