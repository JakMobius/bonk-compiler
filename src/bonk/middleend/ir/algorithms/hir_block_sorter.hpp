#pragma once

#include "bonk/middleend/ir/hir.hpp"
namespace bonk {

class HIRBlockSorter {
  public:
    HIRBlockSorter() = default;

    bool sort(HIRProgram& program);
    bool sort(HIRProcedure& procedure);
};

}