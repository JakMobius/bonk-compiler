
#include "hir_ssa_converter.hpp"
#include <iostream>
#include "bonk/middleend/ir/hir_printer.hpp"
#include "hir_alive_variables_finder.hpp"
#include "hir_dominance_frontier_finder.hpp"

void bonk::HIRSSAConverter::rename_variables(bonk::HIRProcedure& procedure,
                                             HIRDominanceFrontierFinder& df_finder) {
    HIRRenameContext context(df_finder);

    for (int i = 0; i < procedure.parameters.size(); i++) {
        procedure.parameters[i].register_id = context.new_name(procedure.parameters[i].register_id);
    }

    rename(*procedure.base_blocks[procedure.start_block_index], context);

    // Check for unreachable blocks
    for (int i = 0; i < procedure.base_blocks.size(); i++) {
        if (context.visited[i]) {
            continue;
        }

        // This block is unreachable, replace its contents with "ret"
        auto& block = *procedure.base_blocks[i];
        block.instructions.clear();
        block.instructions.push_back(block.instruction<HIRReturnInstruction>());
    }

    procedure.used_registers = context.counter;
}

int bonk::HIRSSAConverter::HIRRenameContext::new_name(int variable) {
    stack[variable].push_back(counter);
    history.push_back(variable);
    return counter++;
}

int bonk::HIRSSAConverter::HIRRenameContext::top(int variable) {
    return stack[variable].back();
}

void bonk::HIRSSAConverter::HIRRenameContext::pop(int count) {
    for (int i = 0; i < count; i++) {
        int variable = history.back();
        history.pop_back();
        stack[variable].pop_back();
    }
}

void bonk::HIRSSAConverter::rename(bonk::HIRBaseBlock& block, HIRRenameContext& context) {

    context.visited[block.index] = true;

    int new_names = 0;

    for (auto& instruction : block.instructions) {
        if (instruction->type != HIRInstructionType::phi_function) {
            break;
        }

        auto* phi = (HIRPhiFunctionInstruction*)instruction;

        new_names++;
        phi->target = context.new_name(phi->target);
    }

    for (auto& instruction : block.instructions) {
        if (instruction->type == HIRInstructionType::phi_function) {
            continue;
        }

        for (int i = instruction->get_read_register_count() - 1; i >= 0; i--) {
            auto& reg = instruction->get_read_register(i);
            reg = context.top(reg);
        }

        for (int i = instruction->get_write_register_count() - 1; i >= 0; i--) {
            auto& reg = instruction->get_write_register(i, nullptr);
            new_names++;
            reg = context.new_name(reg);
        }
    }

    for (auto& successor : block.successors) {
        for (auto& instruction : successor->instructions) {
            if (instruction->type != HIRInstructionType::phi_function) {
                break;
            }

            auto* phi = (HIRPhiFunctionInstruction*)instruction;

            for (int i = 0; i < phi->sources.size(); i++) {
                if (successor->predecessors[i] == &block) {
                    phi->sources[i] = context.top(phi->sources[i]);
                    break;
                }
            }
        }
    }

    for (auto& successor : context.df_finder.dominance_tree_builder.get_children(block.index)) {
        rename(*block.procedure.base_blocks[successor], context);
    }

    context.pop(new_names);
}

void bonk::HIRSSAConverter::convert(bonk::HIRProgram& program) {
    for (auto& procedure : program.procedures) {
        convert(*procedure);
    }
}

void bonk::HIRSSAConverter::convert(bonk::HIRProcedure& procedure) {
    if(procedure.is_external) {
        return;
    }

    HIRDominatorFinder d_finder(procedure);
    HIRDominanceTreeBuilder dt_builder(d_finder);
    HIRDominanceFrontierFinder df_finder(dt_builder);

    insert_phi_functions(procedure, df_finder);
    rename_variables(procedure, df_finder);
}

struct VariableInfo {
    bonk::DynamicBitSet blocks;
    bonk::HIRDataType type;
};

std::vector<VariableInfo> get_variable_info(bonk::HIRProcedure& procedure) {
    std::vector<VariableInfo> blocks;
    blocks.reserve(procedure.used_registers);

    for (int i = 0; i < procedure.used_registers; i++) {
        blocks.push_back(
            {bonk::DynamicBitSet(procedure.base_blocks.size()), bonk::HIRDataType::unset});
    }

    for (auto& block : procedure.base_blocks) {
        for (auto& instruction : block->instructions) {
            int write_registers = instruction->get_write_register_count();

            for (int i = 0; i < write_registers; i++) {
                bonk::HIRDataType type = bonk::HIRDataType::unset;
                auto reg = instruction->get_write_register(i, &type);

                blocks[reg].type = type;
                blocks[reg].blocks[block->index] = true;
            }
        }
    }

    return blocks;
}

void bonk::HIRSSAConverter::insert_phi_functions(bonk::HIRProcedure& procedure,
                                                 HIRDominanceFrontierFinder& df_finder) {
    HIRAliveVariablesFinder av_finder;
    av_finder.walk(procedure);

    auto frontiers = df_finder.get_frontiers();

    auto variable_info = get_variable_info(procedure);

    DynamicBitSet work_list(procedure.base_blocks.size());
    DynamicBitSet visited_list(procedure.base_blocks.size());
    DynamicBitSet phi_functions(procedure.base_blocks.size());

    for (int reg_index = 0; reg_index < variable_info.size(); reg_index++) {
        auto& block_map = variable_info[reg_index];

        work_list = block_map.blocks;
        visited_list.reset();
        phi_functions.reset();

        bool changed = true;

        while (changed) {
            changed = false;
            for (int i = 0; i < work_list.size(); i++) {
                if (!work_list[i]) {
                    continue;
                }
                visited_list[i] = true;
                work_list[i] = false;

                auto frontier = frontiers[i];
                if (frontier == -1) {
                    continue;
                }

                if(!av_finder.get_in(*procedure.base_blocks[frontier])[reg_index]) {
                    continue;
                }

                phi_functions[frontier] = true;

                if (visited_list[frontier]) {
                    continue;
                }
                work_list[frontier] = true;
                changed = true;
            }
        }

        for (int i = 0; i < phi_functions.size(); i++) {
            if (!phi_functions[i]) {
                continue;
            }
            auto& block = procedure.base_blocks[i];
            auto phi = block->instruction<HIRPhiFunctionInstruction>();
            phi->type = variable_info[reg_index].type;
            phi->target = reg_index;

            for (int j = 0; j < block->predecessors.size(); j++) {
                phi->sources.push_back(reg_index);
            }
            block->instructions.insert(block->instructions.begin(), phi);
        }
    }
}