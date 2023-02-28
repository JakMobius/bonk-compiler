#pragma once

namespace bonk {

struct Backend;

}

#include "../compiler.hpp"
#include "../tree/ast.hpp"

namespace bonk {

struct Backend {
    Backend() = default;

    virtual ~Backend() = default;

    virtual bool compile_ast(Compiler* linked_compiler, TreeNodeList* ast,
                             FILE* target);
};

} // namespace bonk
