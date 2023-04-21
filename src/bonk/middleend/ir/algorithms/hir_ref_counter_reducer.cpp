
#include "hir_ref_counter_reducer.hpp"

bool bonk::HIRRefCountReducer::reduce(bonk::HIRProgram& program) {
    for (auto& procedure : program.procedures) {
        if (!reduce(*procedure))
            return false;
    }
    return true;
}

void remove_inc(bonk::HIRBaseBlock& block, bonk::IRRegister register_id, int keep) {
    for (auto it = block.instructions.begin(); it != block.instructions.end();) {
        auto& instruction = *it;
        if (instruction->type != bonk::HIRInstructionType::inc_ref_counter) {
            ++it;
            continue;
        }

        if (instruction->get_read_register(0) != register_id) {
            ++it;
            continue;
        }

        if (keep > 0) {
            ++it;
            keep--;
            continue;
        }

        it = block.instructions.erase(it);
    }
}

void remove_dec(bonk::HIRBaseBlock& block, bonk::IRRegister register_id, int keep) {
    for (auto it = block.instructions.rbegin(); it != block.instructions.rend();) {
        auto& instruction = *it;
        if (instruction->type != bonk::HIRInstructionType::dec_ref_counter) {
            ++it;
            continue;
        }

        if (instruction->get_read_register(0) != register_id) {
            ++it;
            continue;
        }

        if (keep > 0) {
            ++it;
            keep--;
            continue;
        }

        it = std::list<bonk::HIRInstruction*>::reverse_iterator(
            block.instructions.erase(std::next(it).base()));
    }
}

bool bonk::HIRRefCountReducer::reduce(bonk::HIRProcedure& procedure) {
    if(procedure.is_external) {
        return true;
    }

    std::unordered_map<IRRegister, int> balance;

    for (auto& block : procedure.base_blocks) {
        balance.clear();

        for (auto& instruction : block->instructions) {
            if (instruction->type == HIRInstructionType::inc_ref_counter) {
                balance[instruction->get_read_register(0)]++;
            }
            if (instruction->type == HIRInstructionType::dec_ref_counter) {
                balance[instruction->get_read_register(0)]--;
            }
        }

        for (auto [register_id, register_balance] : balance) {
            if (register_balance > 0) {
                remove_dec(*block, register_id, 0);
                remove_inc(*block, register_id, register_balance);
            }
            if (register_balance < 0) {
                remove_inc(*block, register_id, 0);
                remove_dec(*block, register_id, -register_balance);
            }
            if (register_balance == 0) {
                remove_inc(*block, register_id, 1);
                remove_dec(*block, register_id, 1);
            }
        }
    }

    return true;
}
