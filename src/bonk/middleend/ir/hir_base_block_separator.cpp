
#include "hir_base_block_separator.hpp"
#include "../middleend.hpp"

void bonk::HIRBaseBlockSeparator::separate_blocks(bonk::IRProgram& program) {
    for (auto& procedure : program.procedures) {
        separate_blocks(procedure);
    }
}

void bonk::HIRBaseBlockSeparator::separate_blocks(bonk::IRProcedure& procedure) {
    assert(procedure.base_blocks.size() == 1);
    IRBaseBlock block = procedure.base_blocks[0];
    procedure.base_blocks[0].instructions.clear();

    bool is_first_instruction = false;

    IRBaseBlock* current_block = &procedure.base_blocks[0];

    for (auto it = block.instructions.begin(); it != block.instructions.end(); it++) {
        auto hir_instruction = (HIRInstruction*)*it;

        if (hir_instruction->type == HIRInstructionType::label) {
            procedure.create_base_block();
            current_block = &procedure.base_blocks.back();
        } else if (is_first_instruction) {
            // Create a new label for the block, insert it before the first instruction
            int unused_id = procedure.program.id_table.get_unused_id();
            auto label = procedure.instruction<HIRLabel>(unused_id);
            current_block->instructions.insert(it, label);
        }

        current_block->instructions.push_back(hir_instruction);

        if (hir_instruction->type == HIRInstructionType::jump ||
            hir_instruction->type == HIRInstructionType::jump_nz) {
            procedure.create_base_block();
            current_block = &procedure.base_blocks.back();
            is_first_instruction = true;
        }
    }
}
