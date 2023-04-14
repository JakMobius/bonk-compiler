
#include "middleend.hpp"
#include "bonk/middleend/ir/algorithms/hir_base_block_separator.hpp"
#include "bonk/middleend/ir/algorithms/hir_loc_collapser.hpp"
#include "bonk/middleend/ir/algorithms/hir_ref_count_replacer.hpp"
#include "bonk/middleend/ir/algorithms/hir_variable_index_compressor.hpp"

bool bonk::MiddleEnd::do_passes(HIRProgram& program) {

    if(!HIRVariableIndexCompressor().compress(program))
        return false;

    if(!HIRBaseBlockSeparator().separate_blocks(program))
        return false;

    if(!HIRLocCollapser().collapse(program))
        return false;

    if(!HIRRefCountReplacer().replace_ref_counters(program))
        return false;

    return true;
}
