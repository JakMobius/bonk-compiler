
#include "hir_copy_propagation.hpp"

bool bonk::HIRCopyPropagation::propagate_copies(HIRProgram& program) {
    for (auto& procedure : program.procedures) {
        if (!propagate_copies(*procedure)) {
            return false;
        }
    }
    return true;
}

bool bonk::HIRCopyPropagation::propagate_copies(HIRProcedure& procedure) {
    std::unordered_map<IRRegister, HIRInstruction*> definitions;

    for (auto& block : procedure.base_blocks) {
        for (auto& instruction : block->instructions) {
            if (instruction->get_write_register_count() == 1) {
                definitions[instruction->get_write_register(0)] = instruction;
            }
        }
    }

    while (true) {
        bool changed = false;

        for (auto& block : procedure.base_blocks) {
            for (auto instruction : block->instructions) {
                int read_register_count = instruction->get_read_register_count();

                for (int i = 0; i < read_register_count; i++) {
                    auto& read_register = instruction->get_read_register(i);
                    auto constant_it = definitions.find(read_register);

                    if (constant_it == definitions.end()) {
                        continue;
                    }

                    HIRInstruction* definition = constant_it->second;
                    if (definition->type == HIRInstructionType::operation) {
                        auto operation_definition = (HIROperationInstruction*)definition;

                        if (operation_definition->operation_type == HIROperationType::assign) {
                            read_register = operation_definition->left;
                            changed = true;
                        }
                    }
                }
            }
        }

        if (!changed) {
            break;
        }
    }

    return true;
}
