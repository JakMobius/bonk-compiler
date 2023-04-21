
#include "hir_copy_propagation.hpp"
#include "hir_definition_finder.hpp"

bool bonk::HIRCopyPropagation::propagate_copies(HIRProgram& program) {
    for (auto& procedure : program.procedures) {
        if (!propagate_copies(*procedure)) {
            return false;
        }
    }
    return true;
}

bool bonk::HIRCopyPropagation::propagate_copies(HIRProcedure& procedure) {
    if(procedure.is_external) {
        return true;
    }

    auto definitions =
        HIRDefinitionFinder().find_typed_definitions(procedure, HIRInstructionType::operation);

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

                    auto definition = (HIROperationInstruction*)constant_it->second;

                    if (definition->operation_type == HIROperationType::assign) {
                        read_register = definition->left;
                        changed = true;
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
