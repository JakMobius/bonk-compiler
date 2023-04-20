#pragma once

#include "bonk/middleend/ir/hir.hpp"
namespace bonk {

class HIRUnusedDefDeleter {
  public:

    HIRUnusedDefDeleter() = default;

    bool delete_unused_defs(HIRProgram& program);
    bool delete_unused_defs(HIRProcedure& procedure);
};

}