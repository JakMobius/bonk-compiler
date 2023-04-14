#pragma once

#include <unordered_map>
#include <vector>
#include "hir_cfg_walker.hpp"
#include "utils/dynamic_bitset.hpp"

namespace bonk {

class HIRDominatorFinder {

  public:
    HIRDominatorFinder() {
    }

    std::vector<DynamicBitSet> find_dominators(HIRProcedure& procedure);
};

}