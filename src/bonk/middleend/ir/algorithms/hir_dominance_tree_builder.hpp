#pragma once

#include <vector>
#include "../hir.hpp"
#include "hir_dominator_finder.hpp"
#include "utils/dynamic_bitset.hpp"

namespace bonk {

// This class calculates the dominance tree of a procedure lazily
// It is possible to get the parent of a block or the children of a block
// without calculating the whole tree. The get_children method will
// trigger the calculation of the whole tree, however.

class HIRDominanceTreeBuilder {
  public:
    HIRProcedure& procedure;
    HIRDominatorFinder& dominator_finder;

    HIRDominanceTreeBuilder(HIRDominatorFinder& dominator_finder);

    void build();

    int get_parent(int block_index);
    const std::vector<int>& get_children(int block_index);

  private:
    int calculate_parent(int block_index);

    std::vector<int> dominance_tree_parents;
    std::vector<std::vector<int>> dominance_tree_children;
    bool is_built = false;
};

} // namespace bonk