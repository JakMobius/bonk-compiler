
#include "hir_block_sorter.hpp"
#include "hir_dominance_tree_builder.hpp"
#include "hir_dominator_finder.hpp"

bool bonk::HIRBlockSorter::sort(bonk::HIRProgram& program) {
    for (auto& procedure : program.procedures) {
        if (!sort(*procedure))
            return false;
    }
    return true;
}

void walk(bonk::HIRBaseBlock& block, int* order, int& index) {
    if (order[block.index] != -1) {
        return;
    }

    order[block.index] = 0;

    for (auto& child : block.successors) {
        walk(*child, order, index);
    }

    order[block.index] = index--;
}

bool bonk::HIRBlockSorter::sort(bonk::HIRProcedure& procedure) {
    if(procedure.is_external) {
        return true;
    }

    std::vector<int> old_to_new(procedure.base_blocks.size(), -1);

    int index = procedure.base_blocks.size() - 1;
    walk(*procedure.base_blocks[procedure.start_block_index], old_to_new.data(), index);

    // Make sure to include the exit block, even if it's unreachable
    if (old_to_new[procedure.end_block_index] == -1) {
        old_to_new[procedure.end_block_index] = index--;
    }

    // Normalize indices
    index++;

    for (int & i : old_to_new) {
        if (i != -1) {
            i -= index;
        }
    }

    procedure.rename_blocks(old_to_new.data());
    return true;
}