#pragma once

namespace bonk {

struct Backend;
struct Compiler;

}

#include "utils/streams.hpp"
#include "../tree/ast.hpp"

namespace bonk {

struct Backend {
    Compiler& linked_compiler;
    Backend(Compiler& linked_compiler): linked_compiler(linked_compiler) {};

    virtual ~Backend() = default;

    virtual bool compile_ast(std::unique_ptr<TreeNode> ast, const OutputStream& target);
};

} // namespace bonk
