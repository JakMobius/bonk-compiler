#pragma once

#include <memory>
#include "bonk/compiler/compiler.hpp"
#include "bonk/middleend/ir/ir.hpp"

namespace bonk {

class MiddleEnd {
  public:
    Compiler& compiler;
    std::unique_ptr<IRProgram> program;

    MiddleEnd(Compiler& compiler): compiler(compiler) {};

    bool do_passes();

};

}