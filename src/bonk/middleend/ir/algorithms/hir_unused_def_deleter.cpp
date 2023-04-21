
#include "hir_unused_def_deleter.hpp"
#include "utils/dynamic_bitset.hpp"

bool bonk::HIRUnusedDefDeleter::delete_unused_defs(bonk::HIRProgram& program) {
    for (auto& procedure : program.procedures) {
        if (!delete_unused_defs(*procedure)) {
            return false;
        }
    }
    return true;
}

bool bonk::HIRUnusedDefDeleter::delete_unused_defs(bonk::HIRProcedure& procedure) {
    if(procedure.is_external) {
        return true;
    }

    bonk::DynamicBitSet used(procedure.used_registers, false);

    while(true) {
        used.reset();
        bool changed = false;

        for (auto& block : procedure.base_blocks) {
            for (auto& command : block->instructions) {
                int read_register_count = command->get_read_register_count();
                for (int i = 0; i < read_register_count; i++) {
                    used[command->get_read_register(i)] = true;
                }
            }
        }

        for (auto& block : procedure.base_blocks) {
            for (auto it = block->instructions.begin(); it != block->instructions.end();) {
                auto& command = *it;
                if (command->type == HIRInstructionType::call) {
                    // Not removing call instructions
                    ++it;
                    continue;
                }

                if (command->get_write_register_count() == 0) {
                    ++it;
                    continue;
                }

                int write_register = command->get_write_register(0);

                if (used[write_register]) {
                    ++it;
                    continue;
                }

                it = block->instructions.erase(it);
                changed = true;
            }
        }

        if(!changed) {
            break;
        }
    }
    return true;
}
