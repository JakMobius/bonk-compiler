
#include "hir_dominator_finder.hpp"

void bonk::HIRDominatorFinder::calculate_dominators() {

    dominators.reserve(procedure.base_blocks.size());
    for (auto& block : procedure.base_blocks) {
        dominators.emplace_back(procedure.base_blocks.size(), false);
    }

    bonk::DynamicBitSet work_list(procedure.base_blocks.size(), false);
    work_list[procedure.start_block_index] = true;

    bonk::DynamicBitSet temp(procedure.base_blocks.size());

    while (true) {
        bool changed = false;
        for (int i = 0; i < procedure.base_blocks.size(); i++) {
            if (!work_list[i])
                continue;
            work_list[i] = false;

            auto& block = *procedure.base_blocks[i];

            if (!block.predecessors.empty()) {
                temp.set();

                for (auto& predecessor : block.predecessors) {
                    auto& predecessor_dominators = dominators[predecessor->index];
                    if(!predecessor_dominators[predecessor->index]) {
                        // This block is not processed yet
                        continue;
                    }
                    temp &= predecessor_dominators;
                }
            } else {
                temp.reset();
            }

            temp[i] = true;
            if (temp == dominators[i])
                continue;

            dominators[i] = temp;

            changed = true;

            for (auto& successor : block.successors) {
                work_list[successor->index] = true;
            }
        }
        if (!changed) {
            break;
        }
    }
}

std::vector<bonk::DynamicBitSet>& bonk::HIRDominatorFinder::get_dominators() {
    if (dominators.empty()) {
        calculate_dominators();
    }
    return dominators;
}

bonk::HIRDominatorFinder::HIRDominatorFinder(bonk::HIRProcedure& procedure) : procedure(procedure) {
}
