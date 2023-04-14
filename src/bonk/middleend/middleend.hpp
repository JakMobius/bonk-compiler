#pragma once

#include <memory>
#include "bonk/compiler/compiler.hpp"
#include "bonk/middleend/ir/instruction_pool.hpp"

namespace bonk {

class MiddleEnd {
  public:
    Compiler& compiler;

    MiddleEnd(Compiler& compiler): compiler(compiler) {};

    bool do_passes(HIRProgram& program);

};

}