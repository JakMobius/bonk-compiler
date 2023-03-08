#pragma once

#include "bonk/compiler.hpp"
namespace bonk {

class MiddleEnd {
  public:
    Compiler& linked_compiler;
    explicit MiddleEnd(Compiler& linked_compiler) : linked_compiler(linked_compiler){};
    ~MiddleEnd() = default;

    bool run_ast(TreeNode* ast);
};

} // namespace bonk