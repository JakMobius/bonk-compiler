
#include "middleend.hpp"
#include "bonk/middleend/ir/algorithms/hir_base_block_separator.hpp"
#include "bonk/middleend/ir/algorithms/hir_block_sorter.hpp"
#include "bonk/middleend/ir/algorithms/hir_copy_propagation.hpp"
#include "bonk/middleend/ir/algorithms/hir_jmp_reducer.hpp"
#include "bonk/middleend/ir/algorithms/hir_jnz_optimizer.hpp"
#include "bonk/middleend/ir/algorithms/hir_loc_collapser.hpp"
#include "bonk/middleend/ir/algorithms/hir_ref_count_replacer.hpp"
#include "bonk/middleend/ir/algorithms/hir_ref_counter_reducer.hpp"
#include "bonk/middleend/ir/algorithms/hir_ssa_converter.hpp"
#include "bonk/middleend/ir/algorithms/hir_unreachable_code_deleter.hpp"
#include "bonk/middleend/ir/algorithms/hir_unused_def_deleter.hpp"
#include "bonk/middleend/ir/algorithms/hir_variable_index_compressor.hpp"

bool bonk::MiddleEnd::do_passes(HIRProgram& program) {

    bonk::HIRVariableIndexCompressor().compress(program);
    bonk::HIRBaseBlockSeparator().separate_blocks(program);
    bonk::HIRLocCollapser().collapse(program);
    bonk::HIRSSAConverter().convert(program);

    bonk::HIRCopyPropagation().propagate_copies(program);
    bonk::HIRUnusedDefDeleter().delete_unused_defs(program);
    bonk::HIRRefCountReducer().reduce(program);

    bonk::HIRRefCountReplacer().replace_ref_counters(program);

    bonk::HIRJnzOptimizer().optimize(program);
    bonk::HIRUnreachableCodeDeleter().delete_unreachable_code(program);
    bonk::HIRJmpReducer().reduce(program);
    bonk::HIRUnusedDefDeleter().delete_unused_defs(program);
    bonk::HIRVariableIndexCompressor().compress(program);
    bonk::HIRLocCollapser().collapse(program);
    bonk::HIRBlockSorter().sort(program);

    return true;
}
