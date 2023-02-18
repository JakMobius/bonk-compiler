
#include "x86_scope_stack.hpp"

namespace bonk::x86_backend {

scope_stack::scope_stack() {
}

void scope_stack::push_scope(field_list* scope) {
    scopes.push_back(scope);
}

void scope_stack::pop_scope() {
    scopes.pop_back();
}

field_list* scope_stack::top() {
    return scopes[scopes.size() - 1];
}

variable* scope_stack::get_variable(tree_node_identifier* identifier, field_list** scope) {

    for (long long i = scopes.size() - 1; i >= 0; i--) {

        variable* var = scopes[i]->get_variable(identifier);

        if (var != nullptr) {
            if (scope)
                *scope = scopes[i];
            return var;
        }
    }

    return nullptr;
}

} // namespace bonk::x86_backend