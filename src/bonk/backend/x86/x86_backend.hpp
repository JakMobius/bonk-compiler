#pragma once

namespace bonk::x86_backend {

struct Backend;

}

#include "../../compiler.hpp"
#include "../../tree/nodes/nodes.hpp"
#include "bonk/backend/backend.hpp"

namespace bonk::x86_backend {

struct Backend : bonk::Backend {

    Backend()= default;;

    ~Backend() override= default;;

    bool compile_ast(Compiler* linked_compiler, TreeNodeList* ast,
                     FILE* target) override;
};

} // namespace bonk::x86_backend
