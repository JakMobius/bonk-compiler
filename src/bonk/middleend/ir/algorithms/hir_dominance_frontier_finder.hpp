#pragma once

#include <unordered_map>
#include <vector>
#include "hir_cfg_walker.hpp"
#include "hir_dominance_tree_builder.hpp"
#include "utils/dynamic_bitset.hpp"

namespace bonk {

class HIRDominanceFrontierFinder {
  public:
    bonk::HIRProcedure& procedure;
    bonk::HIRDominanceTreeBuilder& dominance_tree_builder;

    explicit HIRDominanceFrontierFinder(bonk::HIRDominanceTreeBuilder& dominance_tree_builder)
        : procedure(dominance_tree_builder.procedure),
          dominance_tree_builder(dominance_tree_builder) {
    }

    std::vector<int>& get_frontiers();

  private:
    std::vector<int> counted_dominators;
    void calculate_dominators();
};

} // namespace bonk