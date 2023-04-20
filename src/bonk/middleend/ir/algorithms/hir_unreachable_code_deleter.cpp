
#include "hir_unreachable_code_deleter.hpp"
#include "utils/dynamic_bitset.hpp"

bool bonk::HIRUnreachableCodeDeleter::delete_unreachable_code(HIRProcedure& procedure) {
    bonk::DynamicBitSet reachable(procedure.base_blocks.size(), false);
    bonk::DynamicBitSet work_list(procedure.base_blocks.size(), false);

    work_list[procedure.start_block_index] = true;

    while (true) {
        bool changed = false;
        for (int i = 0; i < work_list.size(); i++) {
            if (!work_list[i])
                continue;
            changed = true;
            work_list[i] = false;

            if (reachable[i])
                continue;

            reachable[i] = true;

            for (auto& successor : procedure.base_blocks[i]->successors) {
                work_list[successor->index] = true;
            }
        }
        if (!changed)
            break;
    }

    for (int i = 0; i < reachable.size(); i++) {
        if (!reachable[i]) {
            procedure.base_blocks[i]->kill();
        }
    }

    procedure.remove_killed_blocks();

    return true;
}

bool bonk::HIRUnreachableCodeDeleter::delete_unreachable_code(bonk::HIRProgram& program) {
    for (auto& procedure : program.procedures) {
        if(!delete_unreachable_code(*procedure)) {
            return false;
        }
    }
    return true;
}
