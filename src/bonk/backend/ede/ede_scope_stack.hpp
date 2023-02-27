
#pragma once

namespace bonk::ede_backend {

struct ScopeStack;

}

#include <vector>
#include "../../compiler.hpp"
#include "ede_variable.hpp"

namespace bonk::ede_backend {

struct ScopeStack {
    std::vector<FieldList*> scopes{};

    ScopeStack();

    bool push_scope(FieldList* scope);

    void pop_scope();

    FieldList* top();

    unsigned long frame_size();

    Variable* get_variable(TreeNodeIdentifier* identifier, FieldList** scope);
};

} // namespace bonk::ede_backend
