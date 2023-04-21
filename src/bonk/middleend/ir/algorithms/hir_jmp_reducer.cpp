
#include "hir_jmp_reducer.hpp"

bool bonk::HIRJmpReducer::reduce(bonk::HIRProgram& program) {
    for (auto& procedure : program.procedures) {
        if (!reduce(*procedure))
            return false;
    }
    return true;
}

bool bonk::HIRJmpReducer::reduce(bonk::HIRProcedure& procedure) {
    if(procedure.is_external) {
        return true;
    }

    for (auto& block : procedure.base_blocks) {
        if (block->index == procedure.start_block_index ||
            block->index == procedure.end_block_index) {
            continue;
        }

        if (block->predecessors.size() != 1) {
            continue;
        }

        auto predecessor = block->predecessors[0];

        if (predecessor->successors.size() != 1) {
            continue;
        }

        if (predecessor->index == block->index) {
            // This is a loop
            continue;
        }

        // Remove the jump from the predecessor
        predecessor->instructions.pop_back();

        // Move all instructions from the block to the predecessor
        predecessor->instructions.splice(predecessor->instructions.end(), block->instructions);

        // Make sure to keep the successors and predecessors correct
        predecessor->successors = std::move(block->successors);

        for(auto& successor : predecessor->successors) {
            for(auto& successor_predecessor : successor->predecessors) {
                if(successor_predecessor->index == block->index) {
                    successor_predecessor = predecessor;
                }
            }
        }

        // Clear the successors and predecessors of the block
        // before killing it, because otherwise these edges
        // will be marked as killed, which might lead to problems
        block->successors.clear();
        block->predecessors.clear();
        block->kill();
    }

    procedure.remove_killed_blocks();

    return true;
}
