
#include "dominance_frontier_finder.hpp"
#include "hir_dominator_finder.hpp"

std::vector<int>& bonk::HIRDominanceFrontierFinder::find_dominators(bonk::HIRProcedure& procedure) {
    counted_dominators.clear();
    counted_dominators.resize(procedure.base_blocks.size());
    std::fill(counted_dominators.begin(), counted_dominators.end(), -1);

    HIRDominatorFinder dominator_finder;
    dominators = &dominator_finder.find_dominators(procedure);

    for (int i = 0; i < procedure.base_blocks.size(); i++) {
        auto& block = *procedure.base_blocks[i];
        int direct_dominator = get_direct_dominator(i);

        if (block.predecessors.size() < 2)
            continue;

        for (auto predecessor : block.predecessors) {
            auto runner = predecessor->index;
            while (runner != direct_dominator) {
                counted_dominators[runner] = i;
                runner = get_direct_dominator(runner);
            }
        }
    }

    dominators = nullptr;
    return counted_dominators;
}

int bonk::HIRDominanceFrontierFinder::get_direct_dominator(int block_index) {
    auto& all_dominators = (*dominators)[block_index];

    for(int i = all_dominators.size() - 1; i >= 0; i--) {
        if(!all_dominators[i]) continue;
        if(i == block_index) continue;

        auto& dominator_dominators = (*dominators)[i];

        bool is_direct = true;

        // Make sure this dominator dominates all other dominators,
        // except for the current block

        for(int j = 0; j < dominator_dominators.size(); j++) {
            if(j == block_index) continue;

            if(!dominator_dominators[j] && all_dominators[j]) {
                is_direct = false;
                break;
            }
        }

        if(is_direct) {
            return i;
        }
    }
    return -1;
}
