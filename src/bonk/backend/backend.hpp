#pragma once

namespace bonk {

struct backend;

}

#include "../compiler.hpp"
#include "../tree/nodes/nodes.hpp"

namespace bonk {

struct backend {
    backend() = default;

    virtual ~backend() = default;

    virtual bool compile_ast(compiler* linked_compiler, tree_node_list<tree_node*>* ast,
                             FILE* target);
};

} // namespace bonk
