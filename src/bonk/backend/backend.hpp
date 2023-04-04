#pragma once

namespace bonk {

struct Backend;
struct Compiler;

}

#include "../tree/ast.hpp"
#include "bonk/middleend/ir/ir.hpp"
#include "utils/streams.hpp"

namespace bonk {

struct Backend {
    Compiler& linked_compiler;
    Backend(Compiler& linked_compiler): linked_compiler(linked_compiler) {};

    virtual ~Backend() = default;

    virtual void compile_program(IRProgram& program, const bonk::OutputStream& output) = 0;
};

} // namespace bonk
