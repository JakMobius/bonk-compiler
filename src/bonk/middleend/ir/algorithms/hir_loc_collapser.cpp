
#include "hir_loc_collapser.hpp"
#include "bonk/middleend/ir/hir.hpp"

bool bonk::HIRLocCollapser::collapse(bonk::HIRProgram& program) {
    for (auto& procedure : program.procedures) {
        if (!collapse(*procedure))
            return false;
    }
    return true;
}

bool bonk::HIRLocCollapser::collapse(bonk::HIRProcedure& procedure) {
    if(procedure.is_external) {
        return true;
    }

    for (auto& block : procedure.base_blocks) {
        if (!collapse(*block))
            return false;
    }
    return true;
}

bool bonk::HIRLocCollapser::collapse(bonk::HIRBaseBlock& block) {
    if (block.instructions.size() == 0)
        return true;

    for (auto it = block.instructions.begin(); it != block.instructions.end();) {
        auto instruction = *it;
        if (instruction->type != HIRInstructionType::location) {
            ++it;
            continue;
        }

        auto next = it;
        ++next;

        if (next == block.instructions.end()) {
            break;
        }

        auto next_instruction = *next;
        if (next_instruction->type != HIRInstructionType::location) {
            ++it;
            continue;
        }

        block.instructions.erase(it);
        it = next;
    }

    return true;
}
