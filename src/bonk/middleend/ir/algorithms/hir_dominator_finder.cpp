
#include "hir_dominator_finder.hpp"

std::vector<bonk::DynamicBitSet>
bonk::HIRDominatorFinder::find_dominators(bonk::HIRProcedure& procedure) {
    std::vector<bonk::DynamicBitSet> result;
    result.reserve(procedure.base_blocks.size());
    for (auto& block : procedure.base_blocks) {
        result.emplace_back(procedure.base_blocks.size(), true);
    }

    bonk::DynamicBitSet work_list(procedure.base_blocks.size(), false);
    work_list[procedure.start_block_index] = true;

    bonk::DynamicBitSet temp(procedure.base_blocks.size());

    while(true) {
        bool changed = false;
        for (int i = 0; i < procedure.base_blocks.size(); i++) {
            if (!work_list[i])
                continue;
            work_list[i] = false;

            auto& block = *procedure.base_blocks[i];

            if (!block.predecessors.empty()) {
                temp.set();

                for (auto& predecessor : block.predecessors) {
                    auto& predecessor_dominators = result[predecessor->index];
                    temp &= predecessor_dominators;
                }
            } else {
                temp.reset();
            }

            temp[i] = true;
            if (temp == result[i])
                continue;

            result[i] = temp;

            changed = true;

            for (auto& successor : block.successors) {
                work_list[successor->index] = true;
            }
        }
        if(!changed) {
            break;
        }
    }

    return result;
}
