
#include "hir_jnz_optimizer.hpp"
#include "hir_definition_finder.hpp"

bool bonk::HIRJnzOptimizer::optimize(bonk::HIRProgram& program) {
    for (auto& procedure : program.procedures) {
        if (!optimize(*procedure)) {
            return false;
        }
    }
    return true;
}

bool bonk::HIRJnzOptimizer::optimize(bonk::HIRProcedure& procedure) {
    if(procedure.is_external) {
        return true;
    }

    auto definitions =
        HIRDefinitionFinder().find_typed_definitions(procedure, HIRInstructionType::constant_load);

    for (auto& block : procedure.base_blocks) {
        for (auto it = block->instructions.begin(); it != block->instructions.end(); it++) {
            auto instruction = *it;
            if (instruction->type != HIRInstructionType::jump_nz)
                continue;
            auto jnz = (HIRJumpNZInstruction*)instruction;
            auto constant_it = definitions.find(jnz->condition);
            if (constant_it == definitions.end())
                continue;

            auto constant = (HIRConstantLoadInstruction*)constant_it->second;

            auto new_jmp = block->instruction<HIRJumpInstruction>();

            if (constant->constant == 0) {
                // Convert jnz <condition> <nzlabel> <zlabel> to jmp <zlabel>
                new_jmp->label_id = jnz->z_label;

                // Remove the control flow edge to the nzlabel
                block->procedure.remove_control_flow_edge(
                    block.get(), procedure.base_blocks[jnz->nz_label].get());
            } else {
                // Convert jnz <condition> <nzlabel> <zlabel> to jmp <nzlabel>
                new_jmp->label_id = jnz->nz_label;

                // Remove the control flow edge to the zlabel
                block->procedure.remove_control_flow_edge(
                    block.get(), procedure.base_blocks[jnz->z_label].get());
            }

            *it = new_jmp;
        }
    }

    return true;
}
