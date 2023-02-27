#pragma once

namespace bonk {

struct Backend;

}

#include "../compiler.hpp"
#include "../tree/nodes/nodes.hpp"

namespace bonk {

struct Backend {
    Backend() = default;

    virtual ~Backend() = default;

    virtual bool compile_ast(Compiler* linked_compiler, TreeNodeList<TreeNode*>* ast,
                             FILE* target);
};

} // namespace bonk
