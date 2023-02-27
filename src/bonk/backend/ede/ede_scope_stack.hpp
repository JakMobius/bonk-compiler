
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

    void push_scope(FieldList* scope);

    void pop_scope();

    FieldList* top() const;

    unsigned long frame_size();

    Variable* get_variable(TreeNodeIdentifier* identifier, FieldList** scope) const;
};

} // namespace bonk::ede_backend
