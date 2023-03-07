#pragma once

namespace bonk::x86_backend {

struct Backend;

}

#include "../../compiler.hpp"
#include "../../tree/ast.hpp"
#include "bonk/backend/backend.hpp"

namespace bonk::x86_backend {

struct Backend : bonk::Backend {

    Backend(Compiler& linked_compiler): bonk::Backend(linked_compiler) {}
    ~Backend() override = default;

    bool compile_ast(std::unique_ptr<TreeNode> ast, const OutputStream& target) override;
};

} // namespace bonk::x86_backend
