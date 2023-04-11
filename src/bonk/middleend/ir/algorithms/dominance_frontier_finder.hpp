#pragma once

#include <unordered_map>
#include <vector>
#include "hir_cfg_forward_walker.hpp"

namespace bonk {

class HIRDominanceFrontierFinder {
    std::vector<int> counted_dominators;
    std::vector<std::vector<bool>>* dominators = nullptr;

    int get_direct_dominator(int block_index);

  public:
    HIRDominanceFrontierFinder() {
    }

    std::vector<int>& find_dominators(HIRProcedure& procedure);
};

}