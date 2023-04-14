#pragma once

#include <unordered_map>
#include <vector>
#include "hir_cfg_walker.hpp"
#include "utils/dynamic_bitset.hpp"

namespace bonk {

class HIRDominanceFrontierFinder {
    int get_direct_dominator(const std::vector<DynamicBitSet>& dominators, int block_index);

  public:
    HIRDominanceFrontierFinder() {
    }

    std::vector<int> find_dominators(HIRProcedure& procedure);
};

}