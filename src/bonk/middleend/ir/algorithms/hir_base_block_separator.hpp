#pragma once

#include <list>
#include <unordered_map>
#include "bonk/middleend/ir/hir.hpp"

namespace bonk {

class HIRBaseBlockSeparator {
    std::list<HIRInstruction*> current_instructions {};
    std::list<HIRInstruction*>::iterator current_it {};
    std::unordered_map<int, HIRBaseBlock*> block_by_id_map;
    std::unordered_map<HIRBaseBlock*, int> id_by_block_map;
    int return_block_label_id = -1;

    HIRBaseBlock* current_block = nullptr;
    HIRProcedure* current_procedure = nullptr;

    HIRInstruction* peek_instruction();
    HIRInstruction* next_instruction();

    void create_next_block();
    void fill_start_block();
    void fill_procedure_body();
    void fill_return_block();
    void insert_jump_edges();

  public:
    bool separate_blocks(HIRProgram& program);
    bool separate_blocks(HIRProcedure& procedure);
    void annotate_block_with_label(HIRBaseBlock* block, int label);
};

} // namespace bonk