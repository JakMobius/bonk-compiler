
#include "hir_dominance_tree_builder.hpp"

void bonk::HIRDominanceTreeBuilder::build() {
    dominance_tree_children.resize(procedure.base_blocks.size());
    for(const auto & base_block : procedure.base_blocks) {
        auto parent = get_parent(base_block->index);
        if(parent != -1) {
            dominance_tree_children[parent].push_back(base_block->index);
        }
    }
    is_built = true;
}

int bonk::HIRDominanceTreeBuilder::calculate_parent(int block_index) {
    auto& dominators = dominator_finder.get_dominators();
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

bonk::HIRDominanceTreeBuilder::HIRDominanceTreeBuilder(bonk::HIRDominatorFinder &dominator_finder)
    : procedure(dominator_finder.procedure), dominator_finder(dominator_finder) {
    dominance_tree_parents.resize(procedure.base_blocks.size());
    std::fill(dominance_tree_parents.begin(), dominance_tree_parents.end(), -1);
}

int bonk::HIRDominanceTreeBuilder::get_parent(int block_index) {
    if (block_index == procedure.start_block_index) {
        return -1;
    }
    if(dominance_tree_parents[block_index] == -1) {
        dominance_tree_parents[block_index] = calculate_parent(block_index);
    }
    return dominance_tree_parents[block_index];
}

const std::vector<int>& bonk::HIRDominanceTreeBuilder::get_children(int block_index) {
    if (!is_built) {
        build();
    }
    return dominance_tree_children[block_index];
}
