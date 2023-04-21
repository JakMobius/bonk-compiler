#pragma once

#include "bonk/middleend/ir/hir.hpp"
namespace bonk {

class HIRRefCountReducer {
public:
    bool reduce(HIRProgram& program);
    bool reduce(HIRProcedure& procedure);
};

}