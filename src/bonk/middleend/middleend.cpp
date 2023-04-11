
#include "middleend.hpp"
#include "bonk/middleend/ir/algorithms/hir_base_block_separator.hpp"
#include "bonk/middleend/ir/algorithms/hir_loc_collapser.hpp"
#include "bonk/middleend/ir/algorithms/hir_ref_count_replacer.hpp"

bool bonk::MiddleEnd::do_passes() {

    if(!HIRBaseBlockSeparator().separate_blocks(*program))
        return false;

    if(!HIRRefCountReplacer().replace_ref_counters(*program))
        return false;

    if(!HIRLocCollapser().collapse(*program))
        return false;

    return true;
}
