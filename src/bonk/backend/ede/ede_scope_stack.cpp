
#include "ede_scope_stack.hpp"

namespace bonk::ede_backend {

scope_stack::scope_stack() {
}

bool scope_stack::push_scope(field_list* scope) {

    if (scopes.size() > 0) {
        field_list* previous_scope = scopes[scopes.size() - 1];

        scope->byte_offset = previous_scope->byte_offset + previous_scope->frame_size;
    }

    scopes.push_back(scope);
    return true;
}

void scope_stack::pop_scope() {
    scopes.pop_back();
}

field_list* scope_stack::top() {
    return scopes[scopes.size() - 1];
}

unsigned long scope_stack::frame_size() {
    unsigned long scope_offset = 0;
    for (int i = 0; i < scopes.size(); i++) {
        scope_offset += scopes[i]->frame_size;
    }

    return scope_offset;
}

variable* scope_stack::get_variable(tree_node_identifier* identifier, field_list** scope) {

    for (int i = scopes.size() - 1; i >= 0; i--) {

        variable* var = scopes[i]->get_variable(identifier);

        if (var != nullptr) {
            if (scope)
                *scope = scopes[i];
            return var;
        }
    }

    return nullptr;
}

} // namespace bonk::ede_backend