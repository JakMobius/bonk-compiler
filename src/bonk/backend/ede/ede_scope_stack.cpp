
#include "ede_scope_stack.hpp"

namespace bonk::ede_backend {

ScopeStack::ScopeStack() = default;

void ScopeStack::push_scope(FieldList* scope) {

    if (!scopes.empty()) {
        FieldList* previous_scope = scopes[scopes.size() - 1];

        scope->byte_offset = previous_scope->byte_offset + previous_scope->frame_size;
    }

    scopes.push_back(scope);
}

void ScopeStack::pop_scope() {
    scopes.pop_back();
}

FieldList* ScopeStack::top() const {
    return scopes[scopes.size() - 1];
}

unsigned long ScopeStack::frame_size() {
    unsigned long scope_offset = 0;
    for (auto & scope : scopes) {
        scope_offset += scope->frame_size;
    }

    return scope_offset;
}

Variable* ScopeStack::get_variable(TreeNodeIdentifier* identifier, FieldList** scope) const {

    for (int i = scopes.size() - 1; i >= 0; i--) {

        Variable* var = scopes[i]->get_variable(identifier);

        if (var != nullptr) {
            if (scope)
                *scope = scopes[i];
            return var;
        }
    }

    return nullptr;
}

} // namespace bonk::ede_backend