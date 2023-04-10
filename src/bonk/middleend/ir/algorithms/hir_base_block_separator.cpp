
#include "hir_base_block_separator.hpp"
#include "bonk/frontend/frontend.hpp"

bool bonk::HIRBaseBlockSeparator::separate_blocks(bonk::IRProgram& program) {
    for (auto& procedure : program.procedures) {
        if(!separate_blocks(*procedure)) {
            return false;
        }
    }
    return true;
}

bool bonk::HIRBaseBlockSeparator::separate_blocks(bonk::IRProcedure& procedure) {
    assert(procedure.base_blocks.size() == 1);
    IRBaseBlock block = *procedure.base_blocks[0].get();
    procedure.base_blocks[0]->instructions.clear();

    // First block in procedure should not be jumped to,
    // that's why is_first_instruction is initialized to false.
    bool is_first_instruction = false;

    IRBaseBlock* current_block = procedure.base_blocks[0].get();

    for (auto& instruction : block.instructions) {
        auto hir_instruction = (HIRInstruction*)instruction;

        if (hir_instruction->type == HIRInstructionType::label) {
            if (!is_first_instruction) {
                procedure.create_base_block();
                current_block = procedure.base_blocks.back().get();
            }
        } else if (is_first_instruction) {
            // Create a new label for the block, insert it before the first instruction
            int unused_id = procedure.program.id_table.get_unused_id();
            auto label = procedure.instruction<HIRLabel>(unused_id);
            current_block->instructions.push_back(label);
        }
        is_first_instruction = false;

        current_block->instructions.push_back(hir_instruction);

        if (hir_instruction->type == HIRInstructionType::jump ||
            hir_instruction->type == HIRInstructionType::jump_nz ||
            hir_instruction->type == HIRInstructionType::return_op) {
            procedure.create_base_block();
            current_block = procedure.base_blocks.back().get();
            is_first_instruction = true;
        }
    }

    // If last block is empty, remove it
    if (current_block->instructions.empty()) {
        procedure.base_blocks.pop_back();
        current_block = procedure.base_blocks.back().get();
    }

    // If last block misses a return, add one
    if (static_cast<HIRInstruction*>(current_block->instructions.back())->type !=
        HIRInstructionType::return_op) {
        auto return_op = procedure.instruction<HIRReturn>();
        current_block->instructions.push_back(return_op);
    }

    return true;
}
