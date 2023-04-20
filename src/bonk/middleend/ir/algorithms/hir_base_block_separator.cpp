
#include "hir_base_block_separator.hpp"
#include "bonk/frontend/frontend.hpp"

bool bonk::HIRBaseBlockSeparator::separate_blocks(bonk::HIRProgram& program) {
    for (auto& procedure : program.procedures) {
        if (!separate_blocks(*procedure)) {
            return false;
        }
    }
    return true;
}

void bonk::HIRBaseBlockSeparator::fill_start_block() {
    // First block should always be empty
    create_next_block();
}

void bonk::HIRBaseBlockSeparator::fill_procedure_body() {

    bool is_first_instruction = true;

    create_next_block();

    while (auto hir_instruction = next_instruction()) {

        if (hir_instruction->type == HIRInstructionType::label) {
            if (!is_first_instruction)
                create_next_block();
            annotate_block_with_label(current_block,
                                      ((HIRLabelInstruction*)hir_instruction)->label_id);
        } else if (is_first_instruction) {
            // Create a new label for the block, insert it before the first instruction
            int unused_id = current_procedure->program.id_table.get_unused_id();
            auto label = current_procedure->instruction<HIRLabelInstruction>(unused_id);
            current_block->instructions.push_back(label);
            annotate_block_with_label(current_block, unused_id);
        }

        is_first_instruction = false;

        current_block->instructions.push_back(hir_instruction);

        if (hir_instruction->type == HIRInstructionType::jump ||
            hir_instruction->type == HIRInstructionType::jump_nz ||
            hir_instruction->type == HIRInstructionType::return_op) {
            create_next_block();
            is_first_instruction = true;
        }
    }
}

void bonk::HIRBaseBlockSeparator::insert_jump_edges() {

    current_procedure->start_block_index = 0;
    current_procedure->end_block_index = current_procedure->base_blocks.size() - 1;

    // Insert edges
    for (int i = 0; i < current_procedure->base_blocks.size() - 1; i++) {
        auto& block = current_procedure->base_blocks[i];
        auto& instructions = block->instructions;

        if (!instructions.empty()) {
            auto* last_instruction = instructions.back();
            if (last_instruction->type == HIRInstructionType::jump) {
                auto jump = (HIRJumpInstruction*)last_instruction;
                auto target_block = block_by_id_map[jump->label_id];
                current_procedure->add_control_flow_edge(block.get(), target_block);
                continue;
            } else if (last_instruction->type == HIRInstructionType::jump_nz) {
                auto jump = (HIRJumpNZInstruction*)last_instruction;
                auto target_block_z = block_by_id_map[jump->z_label];
                auto target_block_nz = block_by_id_map[jump->nz_label];

                current_procedure->add_control_flow_edge(block.get(), target_block_z);
                current_procedure->add_control_flow_edge(block.get(), target_block_nz);
                continue;
            } else if (last_instruction->type == HIRInstructionType::return_op) {
                current_procedure->add_control_flow_edge(
                    block.get(), current_procedure->base_blocks.back().get());
                continue;
            }
        }

        if (i + 1 == current_procedure->end_block_index) {
            // Add ret instruction
            auto ret = current_procedure->instruction<HIRReturnInstruction>();
            block->instructions.push_back(ret);
        } else {
            auto next_block = current_procedure->base_blocks[i + 1].get();
            current_procedure->add_control_flow_edge(block.get(), next_block);
            // Add jump to the next block
            auto jump = current_procedure->instruction<HIRJumpInstruction>();
            jump->label_id = id_by_block_map[next_block];
            block->instructions.push_back(jump);
        }
    }
}

void bonk::HIRBaseBlockSeparator::fill_return_block() {
    // Create an empty return block, if the last is not empty
    if (!current_block->instructions.empty()) {
        current_procedure->create_base_block();
        current_block = current_procedure->base_blocks.back().get();
    }
}

void bonk::HIRBaseBlockSeparator::rename_and_erase_labels() {
    // Remove all 'label' instructions
    for (auto& block : current_procedure->base_blocks) {
        for (auto it = block->instructions.begin(); it != block->instructions.end();) {
            if ((*it)->type == HIRInstructionType::label) {
                it = block->instructions.erase(it);
            } else {
                ++it;
            }
        }
    }

    // Rename all jump instructions
    for (auto& block : current_procedure->base_blocks) {
        for (auto& instruction : block->instructions) {
            if (instruction->type == HIRInstructionType::jump) {
                auto jump = (HIRJumpInstruction*)instruction;
                jump->label_id = block_by_id_map[jump->label_id]->index;
            } else if (instruction->type == HIRInstructionType::jump_nz) {
                auto jump = (HIRJumpNZInstruction*)instruction;
                jump->z_label = block_by_id_map[jump->z_label]->index;
                jump->nz_label = block_by_id_map[jump->nz_label]->index;
            }
        }
    }
}

bool bonk::HIRBaseBlockSeparator::separate_blocks(bonk::HIRProcedure& procedure) {
    current_procedure = &procedure;
    assert(current_procedure->base_blocks.size() == 1);
    current_instructions = std::move(current_procedure->base_blocks[0].get()->instructions);
    current_it = current_instructions.begin();
    current_procedure->base_blocks.clear();

    fill_start_block();
    fill_procedure_body();
    fill_return_block();
    insert_jump_edges();
    rename_and_erase_labels();

    block_by_id_map.clear();
    id_by_block_map.clear();
    current_instructions.clear();
    current_it = {};
    current_procedure = nullptr;

    return true;
}

bonk::HIRInstruction* bonk::HIRBaseBlockSeparator::next_instruction() {
    auto result = peek_instruction();
    if (result) {
        ++current_it;
    }
    return result;
}

bonk::HIRInstruction* bonk::HIRBaseBlockSeparator::peek_instruction() {
    if (current_it == current_instructions.end())
        return nullptr;
    return (bonk::HIRInstruction*)(*current_it);
}

void bonk::HIRBaseBlockSeparator::create_next_block() {
    current_procedure->create_base_block();
    current_block = current_procedure->base_blocks.back().get();
}

void bonk::HIRBaseBlockSeparator::annotate_block_with_label(bonk::HIRBaseBlock* block, int label) {
    block_by_id_map[label] = block;
    id_by_block_map[block] = label;
}
