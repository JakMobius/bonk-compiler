#pragma once

#include "../../compiler.hpp"
#include "../../tree/nodes/nodes.hpp"
#include "bonk/backend/backend.hpp"
#include "ede_backend_context.hpp"
#include "ede_field_list.hpp"
#include "ede_scope_stack.hpp"

namespace bonk::ede_backend {

struct Backend : bonk::Backend {
    Backend(){};

    virtual ~Backend() {
    }

    bool compile_ast(Compiler* linked_compiler, TreeNodeList<bonk::TreeNode*>* ast,
                     FILE* target) override;
};

} // namespace bonk::ede_backend
