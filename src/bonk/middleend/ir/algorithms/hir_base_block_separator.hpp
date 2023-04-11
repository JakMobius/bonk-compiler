#pragma once

#include <list>
#include <unordered_map>
#include "bonk/middleend/ir/hir.hpp"

namespace bonk {

class HIRBaseBlockSeparator {
    std::list<IRInstruction*> current_instructions {};
    std::list<IRInstruction*>::iterator current_it {};
    std::unordered_map<int, IRBaseBlock*> block_by_id_map;
    std::unordered_map<IRBaseBlock*, int> id_by_block_map;
    int return_block_label_id = -1;

    IRBaseBlock* current_block = nullptr;
    IRProcedure* current_procedure = nullptr;

    HIRInstruction* peek_instruction();
    HIRInstruction* next_instruction();

    void create_next_block();
    void fill_start_block();
    void fill_procedure_body();
    void fill_return_block();
    void insert_jump_edges();

  public:
    bool separate_blocks(IRProgram& program);
    bool separate_blocks(IRProcedure& procedure);
    void annotate_block_with_label(IRBaseBlock* block, int label);
};

} // namespace bonk