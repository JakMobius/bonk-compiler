
#include "hir_dominance_frontier_finder.hpp"
#include "hir_dominator_finder.hpp"

std::vector<int> bonk::HIRDominanceFrontierFinder::find_dominators(bonk::HIRProcedure& procedure) {
    std::vector<int> counted_dominators(procedure.base_blocks.size());
    std::fill(counted_dominators.begin(), counted_dominators.end(), -1);

    auto dominators = HIRDominatorFinder().find_dominators(procedure);

    for (int i = 0; i < procedure.base_blocks.size(); i++) {
        auto& block = *procedure.base_blocks[i];
        int direct_dominator = get_direct_dominator(dominators, i);

        if (block.predecessors.size() < 2)
            continue;

        for (auto predecessor : block.predecessors) {
            auto runner = predecessor->index;
            while (runner != direct_dominator) {
                counted_dominators[runner] = i;
                runner = get_direct_dominator(dominators, runner);
            }
        }
    }

    return counted_dominators;
}

int bonk::HIRDominanceFrontierFinder::get_direct_dominator(const std::vector<bonk::DynamicBitSet>& dominators, int block_index) {
    auto& all_dominators = dominators[block_index];
    bonk::DynamicBitSet temp(all_dominators.size());

    for(int i = all_dominators.size() - 1; i >= 0; i--) {
        if(!all_dominators[i]) continue;
        if(i == block_index) continue;

        auto& dominator_dominators = dominators[i];

        // Make sure this dominator dominates all other dominators,
        // except for the current block

        temp |= all_dominators;
        temp -= dominator_dominators;
        temp[block_index] = false;

        if(!temp.any()) {
            return i;
        }
    }
    return -1;
}
