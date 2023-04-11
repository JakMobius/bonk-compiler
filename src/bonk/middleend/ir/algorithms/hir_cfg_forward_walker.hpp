#pragma once

#include <unordered_set>
#include "bonk/middleend/ir/hir.hpp"
#include "bonk/middleend/ir/instruction_pool.hpp"

namespace bonk {

class HIRCFGForwardWalker {
    std::unordered_set<HIRBaseBlock*> visited_blocks;

  public:
    explicit HIRCFGForwardWalker() {
    }

    bool walk(HIRProcedure& procedure);
    virtual bool walk_block(HIRBaseBlock& block);
};

}