#pragma once

#include "bonk/middleend/ir/hir.hpp"
namespace bonk {

class HIRBaseBlockSeparator {
  public:
    static bool separate_blocks(IRProgram& program);
    static bool separate_blocks(IRProcedure& procedure);
};

} // namespace bonk