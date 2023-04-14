

#include "hir_cfg_walker.hpp"

bool bonk::HIRCFGWalker::walk(bonk::HIRProcedure& procedure) {
    if(walk_backwards) {
        auto& end_block = *procedure.base_blocks[procedure.end_block_index];
        return walk_block(end_block);
    } else {
        auto& start_block = *procedure.base_blocks[procedure.start_block_index];
        return walk_block(start_block);
    }
}

bool bonk::HIRCFGWalker::walk_block(bonk::HIRBaseBlock& block) {
    visited_blocks.insert(&block);

    if(walk_backwards) {
        for (auto& next : block.predecessors) {
            if (visited_blocks.find(next) != visited_blocks.end())
                continue;
            if (!walk_block(*next))
                return false;
        }
    } else {
        for (auto& next : block.successors) {
            if (visited_blocks.find(next) != visited_blocks.end())
                continue;
            if (!walk_block(*next))
                return false;
        }
    }
    return true;
}