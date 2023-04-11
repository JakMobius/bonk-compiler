
#include "hir_dominator_finder.hpp"

bool bonk::HIRDominatorFinder::walk_block(bonk::HIRBaseBlock& block) {
    auto& dominators = counted_dominators[block.index];

    dominators.resize(counted_dominators.size());

    if (!block.predecessors.empty()) {
        dominators = counted_dominators[block.predecessors[0]->index];

        for (int i = 1; i < block.predecessors.size(); i++) {
            auto& predecessor_dominators = counted_dominators[block.predecessors[i]->index];
            if(predecessor_dominators.empty()) continue;

            for (int j = 0; j < dominators.size(); j++) {
                dominators[j] = dominators[j] && predecessor_dominators[j];
            }
        }
    }

    dominators[block.index] = true;

    HIRCFGForwardWalker::walk_block(block);
    return true;
}

std::vector<std::vector<bool>>&
bonk::HIRDominatorFinder::find_dominators(bonk::HIRProcedure& procedure) {
    counted_dominators.clear();
    counted_dominators.resize(procedure.base_blocks.size());
    walk_block(*procedure.base_blocks[procedure.start_block_index]);
    return counted_dominators;
}
