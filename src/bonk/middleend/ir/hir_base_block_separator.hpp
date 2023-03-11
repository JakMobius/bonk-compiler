#pragma once

#include "hir.hpp"
namespace bonk {

class HIRBaseBlockSeparator {
  public:
    static void separate_blocks(IRProgram& program);
    static void separate_blocks(IRProcedure& procedure);
};

} // namespace bonk