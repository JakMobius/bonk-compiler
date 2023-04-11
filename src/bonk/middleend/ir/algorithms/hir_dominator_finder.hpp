#pragma once

#include <unordered_map>
#include <vector>
#include "hir_cfg_forward_walker.hpp"

namespace bonk {

class HIRDominatorFinder : public HIRCFGForwardWalker {
    std::unordered_map<HIRBaseBlock*, std::vector<bool>> counted_dominators;

  public:
    HIRDominatorFinder() : HIRCFGForwardWalker() {
    }

    bool walk_block(HIRBaseBlock& block) override;
};

}