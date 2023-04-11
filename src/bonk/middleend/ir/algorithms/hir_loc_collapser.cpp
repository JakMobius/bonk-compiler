
#include "hir_loc_collapser.hpp"
#include "bonk/middleend/ir/hir.hpp"

bool bonk::HIRLocCollapser::collapse(bonk::IRProgram& program) {
    for (auto& procedure : program.procedures) {
        if (!collapse(*procedure))
            return false;
    }
    return true;
}

bool bonk::HIRLocCollapser::collapse(bonk::IRProcedure& procedure) {
    for (auto& block : procedure.base_blocks) {
        if (!collapse(*block))
            return false;
    }
    return true;
}

bool bonk::HIRLocCollapser::collapse(bonk::IRBaseBlock& block) {
    if (block.instructions.size() == 0)
        return true;

    for (auto it = block.instructions.begin(); it != block.instructions.end();) {
        auto instruction = (HIRInstruction*)*it;
        if (instruction->type != HIRInstructionType::location) {
            ++it;
            continue;
        }

        auto next = it;
        ++next;

        if (next == block.instructions.end()) {
            break;
        }

        auto next_instruction = (HIRInstruction*)*next;
        if (next_instruction->type != HIRInstructionType::location) {
            ++it;
            continue;
        }

        block.instructions.erase(it);
        it = next;
    }

    return true;
}
