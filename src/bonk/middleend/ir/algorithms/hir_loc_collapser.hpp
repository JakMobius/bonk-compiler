#pragma once

#include "bonk/middleend/ir/hir.hpp"
#include "bonk/middleend/ir/instruction_pool.hpp"
namespace bonk {

class HIRLocCollapser {
  public:
    bool collapse(HIRProgram& program);
    bool collapse(HIRProcedure& procedure);
    bool collapse(HIRBaseBlock& block);
};

}