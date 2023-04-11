
#include "hir_dominator_finder.hpp"

bool bonk::HIRDominatorFinder::walk_block(bonk::HIRBaseBlock& block) {



    HIRCFGForwardWalker::walk_block(block);
    return true;
}
