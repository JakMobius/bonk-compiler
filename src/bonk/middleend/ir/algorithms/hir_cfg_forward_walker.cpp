

#include "hir_cfg_forward_walker.hpp"

bool bonk::HIRCFGForwardWalker::walk(bonk::IRProcedure& procedure) {
    auto& start_block = *procedure.base_blocks[0];
    return walk_block(start_block);
}

bool bonk::HIRCFGForwardWalker::walk_block(bonk::IRBaseBlock& block) {
    visited_blocks.insert(&block);

    for(auto& next : block.successors) {
        if(visited_blocks.find(next) != visited_blocks.end()) continue;
        if(!walk_block(*next)) return false;
    }
    return true;
}