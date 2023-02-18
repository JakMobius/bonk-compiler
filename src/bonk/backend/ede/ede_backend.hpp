#pragma once

#include "../../compiler.hpp"
#include "../../tree/nodes/nodes.hpp"
#include "../backend.hpp"
#include "ede_backend_context.hpp"
#include "ede_field_list.hpp"
#include "ede_scope_stack.hpp"

namespace bonk::ede_backend {

struct backend : bonk::backend {
    backend(){};

    virtual ~backend() {
    }

    bool compile_ast(compiler* linked_compiler, tree_node_list<bonk::tree_node*>* ast,
                     FILE* target) override;
};

} // namespace bonk::ede_backend
