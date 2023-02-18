#pragma once

namespace bonk::x86_backend {

struct backend;

}

#include "../../compiler.hpp"
#include "../../tree/nodes/nodes.hpp"
#include "../backend.hpp"

namespace bonk::x86_backend {

struct backend : bonk::backend {

    backend(){};

    virtual ~backend(){};

    bool compile_ast(compiler* linked_compiler, tree_node_list<bonk::tree_node*>* ast,
                     FILE* target) override;
};

} // namespace bonk::x86_backend
