
#include "hir_dominance_frontier_finder.hpp"

void bonk::HIRDominanceFrontierFinder::calculate_dominators() {
    counted_dominators.resize(procedure.base_blocks.size());
    std::fill(counted_dominators.begin(), counted_dominators.end(), -1);

    for (int i = 0; i < procedure.base_blocks.size(); i++) {
        auto& block = *procedure.base_blocks[i];
        int direct_dominator = dominance_tree_builder.get_parent(i);

        if (block.predecessors.size() < 2)
            continue;

        for (auto predecessor : block.predecessors) {
            auto runner = predecessor->index;
            while (runner != direct_dominator && runner != -1) {
                counted_dominators[runner] = i;
                runner = dominance_tree_builder.get_parent(runner);
            }
        }
    }
}

std::vector<int>& bonk::HIRDominanceFrontierFinder::get_frontiers() {
    if (counted_dominators.empty()) {
        calculate_dominators();
    }
    return counted_dominators;
}