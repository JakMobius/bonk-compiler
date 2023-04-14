
#include "hir_ssa_converter.hpp"
#include <iostream>
#include "hir_alive_variables_finder.hpp"
#include "hir_dominance_frontier_finder.hpp"

void bonk::HIRSSAConverter::convert(bonk::HIRProgram& program) {
    for (auto& procedure : program.procedures) {
        convert(*procedure);
    }
}

void bonk::HIRSSAConverter::convert(bonk::HIRProcedure& procedure) {
    HIRAliveVariablesFinder av_finder;
    av_finder.walk(procedure);

    HIRDominanceFrontierFinder df_finder;
    auto frontiers = df_finder.find_dominators(procedure);

    auto blocks = get_blocks_set(procedure);

    // First half is the work list, second half is the visited list
    std::vector<bool> work_list(procedure.base_blocks.size() * 2);

    for (auto& [reg_index, block_map] : blocks) {
        std::copy(block_map.begin(), block_map.end(), work_list.begin());
        std::fill(work_list.begin() + procedure.base_blocks.size(), work_list.end(), false);

        for (int i = 0; i < block_map.size(); i++) {
            if (!work_list[i]) {
                continue;
            }
            work_list[i + blocks.size()] = true;
            auto frontier = frontiers[i];
            if (frontier == -1) {
                continue;
            }
            std::cout << "A phi-function is required for register " << reg_index << " in block "
                      << frontier << "\n";

            if (work_list[frontier + blocks.size()]) {
                continue;
            }
            work_list[frontier] = true;
        }
    }
}
std::unordered_map<bonk::IRRegister, std::vector<bool>>
bonk::HIRSSAConverter::get_blocks_set(bonk::HIRProcedure& procedure) {
    std::unordered_map<bonk::IRRegister, std::vector<bool>> blocks;

    for (auto& block : procedure.base_blocks) {
        for (auto& instruction : block->instructions) {
            int operand_count = instruction->get_operand_count();

            for (int i = 0; i < operand_count; i++) {
                auto reg = instruction->get_operand(i);

                if (blocks.find(reg) == blocks.end()) {
                    blocks[reg] = std::vector<bool>(procedure.base_blocks.size());
                }
                blocks[reg][block->index] = true;
            }
        }
    }

    return blocks;
}
