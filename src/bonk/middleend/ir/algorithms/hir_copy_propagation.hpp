#pragma once

#include "bonk/middleend/ir/hir.hpp"
namespace bonk {

class HIRCopyPropagation {
public:
  HIRCopyPropagation() {}

    bool propagate_copies(HIRProgram& program);
    bool propagate_copies(HIRProcedure& procedure);
};

}