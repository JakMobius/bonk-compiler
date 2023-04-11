#pragma once

#include <unordered_set>
#include "bonk/middleend/ir/ir.hpp"

namespace bonk {

class HIRCFGForwardWalker {
    std::unordered_set<IRBaseBlock*> visited_blocks;

  public:
    explicit HIRCFGForwardWalker() {
    }

    bool walk(IRProcedure& procedure);
    bool walk_block(IRBaseBlock& block);
};

}