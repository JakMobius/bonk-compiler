
#include "hir_definition_finder.hpp"

std::unordered_map<bonk::IRRegister, bonk::HIRInstruction*>
bonk::HIRDefinitionFinder::find_definitions(bonk::HIRProcedure& procedure) {
    auto result = std::unordered_map<IRRegister, HIRInstruction*>();

    for (auto& block : procedure.base_blocks) {
        for (auto& instruction : block->instructions) {
            if (instruction->get_write_register_count() == 1) {
                result[instruction->get_write_register(0)] = instruction;
            }
        }
    }

    return result;
}

std::unordered_map<bonk::IRRegister, bonk::HIRInstruction*>
bonk::HIRDefinitionFinder::find_typed_definitions(bonk::HIRProcedure& procedure, bonk::HIRInstructionType type) {
    auto result = std::unordered_map<IRRegister, HIRInstruction*>();

    for (auto& block : procedure.base_blocks) {
        for (auto& instruction : block->instructions) {
            if (instruction->type == type && instruction->get_write_register_count() == 1) {
                result[instruction->get_write_register(0)] = instruction;
            }
        }
    }

    return result;
}

