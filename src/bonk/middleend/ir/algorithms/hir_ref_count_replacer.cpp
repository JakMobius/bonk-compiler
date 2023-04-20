
#include "hir_ref_count_replacer.hpp"
#include "bonk/frontend/annotators/basic_symbol_annotator.hpp"

bool bonk::HIRRefCountReplacer::replace_ref_counters(bonk::HIRProgram& program) {
    for (auto& procedure : program.procedures) {
        if (!replace_ref_counters(*procedure)) {
            return false;
        }
    }
    return true;
}

bool bonk::HIRRefCountReplacer::replace_ref_counters(bonk::HIRProcedure& procedure) {
    current_procedure = &procedure;
    current_program = &procedure.program;
    for (int i = 0; i < procedure.base_blocks.size(); i++) {
        current_base_block = procedure.base_blocks[i].get();
        current_instruction_iterator = current_base_block->instructions.begin();
        while (current_instruction_iterator != current_base_block->instructions.end()) {
            replace_ref_counters((HIRInstruction*)(*current_instruction_iterator));
        }
    }
    current_program = nullptr;
    current_procedure = nullptr;
    return true;
}

void bonk::HIRRefCountReplacer::replace_ref_counters(HIRInstruction* instruction) {

    if (instruction->type == HIRInstructionType::inc_ref_counter) {
        remove_instruction();
        auto inc_instruction = (HIRIncRefCounterInstruction*)instruction;
        increase_reference_count(inc_instruction->address);
    } else if (instruction->type == HIRInstructionType::dec_ref_counter) {
        remove_instruction();
        auto dec_instruction = (HIRDecRefCounterInstruction*)instruction;
        decrease_reference_count(dec_instruction->address, dec_instruction->hive_definition);
    } else {
        current_instruction_iterator++;
    }
}

bonk::IRRegister bonk::HIRRefCountReplacer::get_reference_address(IRRegister hive_register) {
    int constant_register = current_procedure->get_unused_register();
    add_instruction<HIRConstantLoadInstruction>(constant_register, (int64_t)8);

    int value_register = current_procedure->get_unused_register();
    auto instruction = add_instruction<HIROperationInstruction>();
    instruction->target = value_register;
    instruction->result_type = HIRDataType::dword;
    instruction->operand_type = HIRDataType::dword;
    instruction->operation_type = HIROperationType::minus;
    instruction->left = hive_register;
    instruction->right = constant_register;

    return value_register;
}

void bonk::HIRRefCountReplacer::increase_reference_count(bonk::IRRegister register_id) {

    auto skip_block = split_block();
    auto start_block = create_block();

    // If reference is null, skip
    jmpnz(register_id, start_block, skip_block);

    switch_to_block(start_block);
    IRRegister reference_address = get_reference_address(register_id);
    IRRegister reference_counter = load_reference_count(reference_address);
    IRRegister adjusted_reference_counter = adjust_reference_count(reference_counter, 1);
    write_reference_count(reference_address, adjusted_reference_counter);
    jmp(skip_block);

    // Continue on iterating the procedure
    switch_to_block(skip_block);
}

void bonk::HIRRefCountReplacer::decrease_reference_count(bonk::IRRegister register_id,
                                                         TreeNodeHiveDefinition* hive_definition) {
    auto skip_block = split_block();
    auto start_block = create_block();
    auto destruct_block = create_block();
    auto keep_block = create_block();

    // If reference is null, skip everything
    jmpnz(register_id, start_block, skip_block);

    // Fill up the start block
    switch_to_block(start_block);

    IRRegister reference_address = get_reference_address(register_id);
    IRRegister reference_counter = load_reference_count(reference_address);
    IRRegister decreased_reference_counter = adjust_reference_count(reference_counter, -1);

    // If reference counter is zero, call the destructor

    jmpnz(decreased_reference_counter, keep_block, destruct_block);

    // Destruct branch:
    switch_to_block(destruct_block);
    call_destructor(hive_definition, register_id);
    jmp(skip_block);

    // Keep branch:
    // Store the updated reference counter only if it's not zero,
    // otherwise the destructor will call itself infinitely
    switch_to_block(keep_block);
    write_reference_count(reference_address, decreased_reference_counter);
    jmp(skip_block);

    // Continue on iterating the procedure
    switch_to_block(skip_block);
}

void bonk::HIRRefCountReplacer::remove_instruction() {
    current_instruction_iterator =
        current_base_block->instructions.erase(current_instruction_iterator);
}

bonk::IRRegister
bonk::HIRRefCountReplacer::load_reference_count(bonk::IRRegister reference_address) {
    int reference_counter_register = current_procedure->get_unused_register();
    auto load_instruction = add_instruction<HIRMemoryLoadInstruction>();
    load_instruction->target = reference_counter_register;
    load_instruction->type = HIRDataType::dword;
    load_instruction->address = reference_address;
    return reference_counter_register;
}

bonk::IRRegister bonk::HIRRefCountReplacer::adjust_reference_count(IRRegister reference_count,
                                                                   int64_t delta) {
    int constant_register = current_procedure->get_unused_register();
    add_instruction<HIRConstantLoadInstruction>(constant_register, delta);

    int result_register = current_procedure->get_unused_register();
    auto instruction = add_instruction<HIROperationInstruction>();
    instruction->target = result_register;
    instruction->result_type = HIRDataType::dword;
    instruction->operand_type = HIRDataType::dword;
    instruction->operation_type = HIROperationType::plus;
    instruction->left = reference_count;
    instruction->right = constant_register;

    return result_register;
}

void bonk::HIRRefCountReplacer::write_reference_count(bonk::IRRegister reference_address,
                                                      bonk::IRRegister value) {
    auto store_instruction = add_instruction<HIRMemoryStoreInstruction>();
    store_instruction->type = HIRDataType::dword;
    store_instruction->address = reference_address;
    store_instruction->value = value;
}

void bonk::HIRRefCountReplacer::call_destructor(TreeNodeHiveDefinition* hive_definition,
                                                IRRegister register_id) {
    std::string destructor_name =
        std::string(hive_definition->hive_name->identifier_text) + "$$destructor";

    // Find the destructor symbol
    auto scope = current_program->symbol_table.get_scope_for_node(hive_definition)->parent_scope;
    auto destructor_definition = ScopedNameResolver(scope).get_name_definition(destructor_name);
    auto destructor_definition_id = current_program->id_table.get_id(destructor_definition);

    auto destructor_call_parameter = add_instruction<HIRParameterInstruction>();
    destructor_call_parameter->type = HIRDataType::dword;
    destructor_call_parameter->parameter = register_id;

    auto destructor_call_instruction = add_instruction<HIRCallInstruction>();
    destructor_call_instruction->procedure_label_id = destructor_definition_id;
    destructor_call_instruction->return_type = HIRDataType::word;
}

void bonk::HIRRefCountReplacer::switch_to_block(HIRBaseBlock* block) {
    current_base_block = block;
    current_instruction_iterator = current_base_block->instructions.end();
}

bonk::HIRBaseBlock* bonk::HIRRefCountReplacer::create_block() {
    current_procedure->create_base_block();
    return current_procedure->base_blocks.back().get();
}

bonk::HIRBaseBlock* bonk::HIRRefCountReplacer::split_block() {
    auto block = create_block();

    block->instructions.splice(block->instructions.begin(), current_base_block->instructions,
                               current_instruction_iterator,
                               current_base_block->instructions.end());
    current_instruction_iterator = current_base_block->instructions.end();

    // Relink the control flow

    block->successors = current_base_block->successors;
    current_base_block->successors.clear();

    for(auto successor : block->successors) {
        for(auto& predecessor : successor->predecessors) {
            if(predecessor == current_base_block) {
                predecessor = block;
            }
        }
    }

    return block;
}

void bonk::HIRRefCountReplacer::jmpnz(bonk::IRRegister register_id, bonk::HIRBaseBlock* nz_block,
                                      bonk::HIRBaseBlock* z_block) {
    add_instruction<HIRJumpNZInstruction>(register_id, nz_block->index, z_block->index);
    current_base_block->procedure.add_control_flow_edge(current_base_block, z_block);
    current_base_block->procedure.add_control_flow_edge(current_base_block, nz_block);
}

void bonk::HIRRefCountReplacer::jmp(bonk::HIRBaseBlock* block) {
    add_instruction<HIRJumpInstruction>(block->index);
    current_base_block->procedure.add_control_flow_edge(current_base_block, block);
}
