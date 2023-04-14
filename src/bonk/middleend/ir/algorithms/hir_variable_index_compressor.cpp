
#include "hir_variable_index_compressor.hpp"
#include <unordered_map>

bool bonk::HIRVariableIndexCompressor::compress(bonk::HIRProgram& program) {
    for (auto& procedure : program.procedures) {
        compress(*procedure);
    }
    return true;
}

std::unordered_map<bonk::IRRegister, bonk::IRRegister>
bonk::HIRVariableIndexCompressor::compress(bonk::HIRProcedure& procedure) {
    std::unordered_map<bonk::IRRegister, bonk::IRRegister> register_map;
    bonk::IRRegister index = 0;

    for (auto& block : procedure.base_blocks) {
        for (auto& command : block->instructions) {
            int operand_count = command->get_operand_count();
            for (int i = 0; i < operand_count; i++) {
                auto reg = command->get_operand(i);
                if (register_map.find(reg) == register_map.end()) {
                    register_map[reg] = index++;
                }
            }
        }
    }

    for(auto& parameter : procedure.parameters) {
        if(register_map.find(parameter.register_id) == register_map.end()) {
            register_map[parameter.register_id] = index++;
        }
    }

    for (auto& block : procedure.base_blocks) {
        for (auto& command : block->instructions) {
            int operand_count = command->get_operand_count();
            for (int i = 0; i < operand_count; i++) {
                auto reg = command->get_operand(i);
                auto it = register_map.find(reg);
                assert(it != register_map.end());
                command->get_operand(i) = it->second;
            }
        }
    }

    for(auto& parameter : procedure.parameters) {
        parameter.register_id = register_map[parameter.register_id];
    }

    procedure.used_registers = index;
    return register_map;
}