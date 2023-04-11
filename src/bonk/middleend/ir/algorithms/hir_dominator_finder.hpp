#pragma once

#include <unordered_map>
#include <vector>
#include "hir_cfg_forward_walker.hpp"

namespace bonk {

class HIRDominatorFinder : public HIRCFGForwardWalker {
    std::vector<std::vector<bool>> counted_dominators;

    bool walk_block(HIRBaseBlock& block) override;

  public:
    HIRDominatorFinder() : HIRCFGForwardWalker() {
    }

    std::vector<std::vector<bool>>& find_dominators(HIRProcedure& procedure);
};

}