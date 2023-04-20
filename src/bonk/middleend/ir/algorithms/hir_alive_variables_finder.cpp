
#include "hir_alive_variables_finder.hpp"

bool bonk::HIRAliveVariablesFinder::walk(bonk::HIRProcedure& procedure) {
    use.clear();
    define.clear();
    out.clear();

    use.reserve(procedure.base_blocks.size());
    define.reserve(procedure.base_blocks.size());
    out.reserve(procedure.base_blocks.size());

    for (auto& block : procedure.base_blocks) {
        use.emplace_back(procedure.used_registers, false);
        define.emplace_back(procedure.used_registers, false);
        out.emplace_back(procedure.used_registers, false);
    }

    for (auto& block : procedure.base_blocks) {
        update_use_define(*block);
    }

    bonk::DynamicBitSet work_list(procedure.base_blocks.size(), false);
    for (auto& block : procedure.base_blocks) {
        if (block->index != procedure.end_block_index)
            work_list[block->index] = true;
    }

    while (true) {
        bool changed = false;
        for (int i = 0; i < work_list.size(); i++) {
            if (!work_list[i])
                continue;
            changed = true;
            work_list[i] = false;

            auto new_out = get_out(*procedure.base_blocks[i]);
            if (new_out != out[i]) {
                out[i] = new_out;
                for (auto& predecessor : procedure.base_blocks[i]->predecessors) {
                    work_list[predecessor->index] = true;
                }
            }
        }
        if (!changed)
            break;
    }

    return true;
}

void bonk::HIRAliveVariablesFinder::update_use_define(bonk::HIRBaseBlock& block) {
    auto& block_use = use[block.index];
    auto& block_define = define[block.index];

    for (auto& instruction : block.instructions) {
        int read_registers = instruction->get_read_register_count();
        int write_registers = instruction->get_write_register_count();

        for (int i = 0; i < read_registers; i++) {
            auto reg = instruction->get_read_register(i);
            if (!block_define[reg])
                block_use[reg] = true;
        }

        for (int i = 0; i < write_registers; i++) {
            auto reg = instruction->get_write_register(i, nullptr);
            if (!block_use[reg])
                block_define[reg] = true;
        }
    }
}

bonk::DynamicBitSet bonk::HIRAliveVariablesFinder::get_out(bonk::HIRBaseBlock& block) {
    auto& successors = block.successors;
    bonk::DynamicBitSet new_out(block.procedure.used_registers, false);

    for (auto& successor : successors) {
        auto& succ_use = use[successor->index];
        auto& succ_define = define[successor->index];
        auto& succ_out = out[successor->index];

        new_out |= succ_use;
        new_out |= (succ_out - succ_define);
    }

    return new_out;
}

bonk::DynamicBitSet bonk::HIRAliveVariablesFinder::get_in(bonk::HIRBaseBlock& block) {
    auto& block_use = use[block.index];
    auto& block_define = define[block.index];
    auto& block_out = out[block.index];

    auto new_in = bonk::DynamicBitSet(block.procedure.used_registers, false);

    new_in |= block_out;
    new_in -= block_define;
    new_in |= block_use;

    return new_in;
}
