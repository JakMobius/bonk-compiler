
#include "x86_scope_stack.hpp"

namespace bonk::x86_backend {

ScopeStack::ScopeStack() = default;

void ScopeStack::push_scope(FieldList* scope) {
    scopes.push_back(scope);
}

void ScopeStack::pop_scope() {
    scopes.pop_back();
}

FieldList* ScopeStack::top() {
    return scopes[scopes.size() - 1];
}

Variable* ScopeStack::get_variable(TreeNodeIdentifier* identifier, FieldList** scope) {

    for (long long i = scopes.size() - 1; i >= 0; i--) {

        Variable* var = scopes[i]->get_variable(identifier);

        if (var != nullptr) {
            if (scope)
                *scope = scopes[i];
            return var;
        }
    }

    return nullptr;
}

} // namespace bonk::x86_backend