
#include "ede_scope_stack.hpp"

namespace bonk::ede_backend {

ScopeStack::ScopeStack() = default;

bool ScopeStack::push_scope(FieldList* scope) {

    if (scopes.size() > 0) {
        FieldList* previous_scope = scopes[scopes.size() - 1];

        scope->byte_offset = previous_scope->byte_offset + previous_scope->frame_size;
    }

    scopes.push_back(scope);
    return true;
}

void ScopeStack::pop_scope() {
    scopes.pop_back();
}

FieldList* ScopeStack::top() {
    return scopes[scopes.size() - 1];
}

unsigned long ScopeStack::frame_size() {
    unsigned long scope_offset = 0;
    for (int i = 0; i < scopes.size(); i++) {
        scope_offset += scopes[i]->frame_size;
    }

    return scope_offset;
}

Variable* ScopeStack::get_variable(TreeNodeIdentifier* identifier, FieldList** scope) {

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