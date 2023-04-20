#pragma once

#include "bonk/middleend/ir/hir.hpp"

namespace bonk {

class HIRUnreachableCodeDeleter {
  public:

    HIRUnreachableCodeDeleter() {}

    bool delete_unreachable_code(HIRProgram& program);
    bool delete_unreachable_code(HIRProcedure& procedure);
};

}