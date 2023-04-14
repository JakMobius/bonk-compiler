#pragma once

#include <unordered_set>
#include "bonk/middleend/ir/hir.hpp"
#include "bonk/middleend/ir/instruction_pool.hpp"

namespace bonk {

class HIRCFGWalker {
  protected:
    std::unordered_set<HIRBaseBlock*> visited_blocks;

  public:
    explicit HIRCFGWalker() {
    }

    bool walk(HIRProcedure& procedure);
    virtual bool walk_block(HIRBaseBlock& block);
    bool walk_backwards = false;
};

}