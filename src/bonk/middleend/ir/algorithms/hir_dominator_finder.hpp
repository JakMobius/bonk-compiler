#pragma once

#include <unordered_map>
#include <vector>
#include "hir_cfg_walker.hpp"
#include "utils/dynamic_bitset.hpp"

namespace bonk {

class HIRDominatorFinder {
  public:
    HIRProcedure& procedure;

    explicit HIRDominatorFinder(HIRProcedure& procedure);

    std::vector<DynamicBitSet>& get_dominators();

  private:
    void calculate_dominators();

    std::vector<DynamicBitSet> dominators;
};

}