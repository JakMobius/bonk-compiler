#pragma once

namespace bonk {

struct Backend;
struct Compiler;

}

#include "bonk/frontend/ast/ast.hpp"
#include "bonk/middleend/ir/hir.hpp"
#include "utils/streams.hpp"

namespace bonk {

struct Backend {
    Compiler& linked_compiler;
    Backend(Compiler& linked_compiler): linked_compiler(linked_compiler) {};

    virtual ~Backend() = default;

    virtual void compile_program(HIRProgram& program, const bonk::OutputStream& output) = 0;
};

} // namespace bonk
