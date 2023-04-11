
#include "hir_base_block_separator.hpp"
#include "bonk/frontend/frontend.hpp"

bool bonk::HIRBaseBlockSeparator::separate_blocks(bonk::IRProgram& program) {
    for (auto& procedure : program.procedures) {
        if (!separate_blocks(*procedure)) {
            return false;
        }
    }
    return true;
}

void bonk::HIRBaseBlockSeparator::fill_start_block() {
    create_next_block();

    // First block in procedure should not contain any real instructions,
    // only procedure, file and location instructions

    while (true) {
        auto hir_instruction = peek_instruction();
        if (!hir_instruction)
            break;
        switch (hir_instruction->type) {
        case HIRInstructionType::procedure:
        case HIRInstructionType::file:
        case HIRInstructionType::location:
            current_block->instructions.push_back(hir_instruction);
            break;
        default:
            return;
        }
        next_instruction();
    }
}

void bonk::HIRBaseBlockSeparator::fill_procedure_body() {

    bool is_first_instruction = true;

    create_next_block();

    while (auto hir_instruction = next_instruction()) {

        if (hir_instruction->type == HIRInstructionType::label) {
            if (!is_first_instruction)
                create_next_block();
            annotate_block_with_label(current_block, ((HIRLabel*)hir_instruction)->label_id);
        } else if (is_first_instruction) {
            // Create a new label for the block, insert it before the first instruction
            int unused_id = current_procedure->program.id_table.get_unused_id();
            auto label = current_procedure->instruction<HIRLabel>(unused_id);
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
    // Insert edges
    for (int i = 0; i < current_procedure->base_blocks.size() - 1; i++) {
        auto& block = current_procedure->base_blocks[i];
        auto& instructions = block->instructions;

        if(!instructions.empty()) {
            auto* last_instruction = (HIRInstruction*)instructions.back();
            if (last_instruction->type == HIRInstructionType::jump) {
                auto jump = (HIRJump*)last_instruction;
                auto target_block = block_by_id_map[jump->label_id];
                current_procedure->add_control_flow_edge(block.get(), target_block);
                continue;
            } else if (last_instruction->type == HIRInstructionType::jump_nz) {
                auto jump = (HIRJumpNZ*)last_instruction;
                auto target_block_z = block_by_id_map[jump->z_label];
                auto target_block_nz = block_by_id_map[jump->nz_label];

                current_procedure->add_control_flow_edge(block.get(), target_block_z);
                current_procedure->add_control_flow_edge(block.get(), target_block_nz);
                continue;
            } else if (last_instruction->type == HIRInstructionType::return_op) {
                // Replace return with jump to return block
                block->instructions.pop_back();
                auto jump = current_procedure->instruction<HIRJump>();
                jump->label_id = return_block_label_id;
                block->instructions.push_back(jump);

                current_procedure->add_control_flow_edge(
                    block.get(), current_procedure->base_blocks.back().get());
                continue;
            }
        }

        auto next_block = current_procedure->base_blocks[i + 1].get();
        current_procedure->add_control_flow_edge(block.get(), next_block);
        // Add jump to the next block
        auto jump = current_procedure->instruction<HIRJump>();
        jump->label_id = id_by_block_map[next_block];
        block->instructions.push_back(jump);
    }
}

void bonk::HIRBaseBlockSeparator::fill_return_block() {
    // Create an empty return block, if the last is not empty
    if (!current_block->instructions.empty()) {
        current_procedure->create_base_block();
        current_block = current_procedure->base_blocks.back().get();
    }

    // Add a label to the last block
    return_block_label_id = current_procedure->program.id_table.get_unused_id();
    auto label = current_procedure->instruction<HIRLabel>(return_block_label_id);
    current_block->instructions.push_back(label);
    block_by_id_map[return_block_label_id] = current_block;

    // Add a return instruction to the last block
    current_block->instructions.push_back(current_procedure->instruction<HIRReturn>());
}

bool bonk::HIRBaseBlockSeparator::separate_blocks(bonk::IRProcedure& procedure) {
    current_procedure = &procedure;
    assert(current_procedure->base_blocks.size() == 1);
    current_instructions = std::move(current_procedure->base_blocks[0].get()->instructions);
    current_it = current_instructions.begin();
    current_procedure->base_blocks.clear();

    fill_start_block();
    fill_procedure_body();
    fill_return_block();
    insert_jump_edges();

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

void bonk::HIRBaseBlockSeparator::annotate_block_with_label(bonk::IRBaseBlock* block, int label) {
    block_by_id_map[label] = block;
    id_by_block_map[block] = label;
}
