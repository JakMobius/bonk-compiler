#pragma once

#include "bonk/middleend/ir/ir.hpp"
namespace bonk {

class HIRLocCollapser {
  public:
    bool collapse(IRProgram& program);
    bool collapse(IRProcedure& procedure);
    bool collapse(IRBaseBlock& block);
};

}