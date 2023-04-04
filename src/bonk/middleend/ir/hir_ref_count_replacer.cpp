
#include "hir_ref_count_replacer.hpp"
#include "bonk/middleend/annotators/basic_symbol_annotator.hpp"

void bonk::HIRRefCountReplacer::replace_ref_counters(bonk::IRProgram& program) {
    for (auto& procedure : program.procedures) {
        replace_ref_counters(*procedure);
    }
}

void bonk::HIRRefCountReplacer::replace_ref_counters(bonk::IRProcedure& procedure) {
    for (auto& block : procedure.base_blocks) {
        current_base_block = block.get();
        current_instruction_iterator = block->instructions.begin();
        while (current_instruction_iterator != block->instructions.end()) {
            replace_ref_counters((HIRInstruction*)(*current_instruction_iterator));
        }
    }
}

void bonk::HIRRefCountReplacer::replace_ref_counters(HIRInstruction* instruction) {
    if (instruction->type == HIRInstructionType::inc_ref_counter) {
        remove_instruction();
        auto inc_instruction = (HIRIncRefCounter*)instruction;
        increase_reference_count(inc_instruction->address);
    } else if (instruction->type == HIRInstructionType::dec_ref_counter) {
        remove_instruction();
        auto dec_instruction = (HIRDecRefCounter*)instruction;
        decrease_reference_count(dec_instruction->address, dec_instruction->hive_definition);
    } else {
        current_instruction_iterator++;
    }
}

bonk::IRRegister bonk::HIRRefCountReplacer::get_reference_address(IRRegister hive_register) {
    int constant_register = middle_end.id_table.get_unused_id();
    add_instruction<HIRConstantLoad>(constant_register, (int64_t)8);

    int value_register = middle_end.id_table.get_unused_id();
    auto instruction = add_instruction<HIROperation>();
    instruction->target = value_register;
    instruction->result_type = HIRDataType::dword;
    instruction->operand_type = HIRDataType::dword;
    instruction->operation_type = HIROperationType::minus;
    instruction->left = hive_register;
    instruction->right = constant_register;

    return value_register;
}

void bonk::HIRRefCountReplacer::increase_reference_count(bonk::IRRegister register_id) {

    int skip_label = middle_end.id_table.get_unused_id();
    int start_label = middle_end.id_table.get_unused_id();

    // If reference is null, skip
    add_instruction<HIRJumpNZ>(register_id, start_label, skip_label);

    add_instruction<HIRLabel>(start_label);

    IRRegister reference_address = get_reference_address(register_id);
    IRRegister reference_counter = load_reference_count(reference_address);
    IRRegister adjusted_reference_counter = adjust_reference_count(reference_counter, 1);
    write_reference_count(reference_address, adjusted_reference_counter);

    add_instruction<HIRLabel>(skip_label);
}

void bonk::HIRRefCountReplacer::decrease_reference_count(bonk::IRRegister register_id,
                                                         TreeNodeHiveDefinition* hive_definition) {

    int start_label = middle_end.id_table.get_unused_id();
    int destruct_label = middle_end.id_table.get_unused_id();
    int keep_label = middle_end.id_table.get_unused_id();
    int skip_label = middle_end.id_table.get_unused_id();

    // If reference is null, skip
    add_instruction<HIRJumpNZ>(register_id, start_label, skip_label);
    add_instruction<HIRLabel>(start_label);

    IRRegister reference_address = get_reference_address(register_id);
    IRRegister reference_counter = load_reference_count(reference_address);
    IRRegister decreased_reference_counter = adjust_reference_count(reference_counter, -1);

    // If reference counter is zero, call the destructor

    add_instruction<HIRJumpNZ>(decreased_reference_counter, keep_label, destruct_label);

    // Destruct branch:
    add_instruction<HIRLabel>(destruct_label);
    call_destructor(hive_definition, register_id);
    add_instruction<HIRJump>()->label_id = skip_label;

    // Keep branch:
    // Store the updated reference counter only if it's not zero,
    // otherwise the destructor will call itself infinitely
    add_instruction<HIRLabel>(keep_label);
    write_reference_count(reference_address, decreased_reference_counter);

    add_instruction<HIRLabel>(skip_label);
}

void bonk::HIRRefCountReplacer::remove_instruction() {
    current_instruction_iterator =
        current_base_block->instructions.erase(current_instruction_iterator);
}

bonk::IRRegister bonk::HIRRefCountReplacer::load_reference_count(bonk::IRRegister reference_address) {
    int reference_counter_register = middle_end.id_table.get_unused_id();
    auto load_instruction = add_instruction<HIRMemoryLoad>();
    load_instruction->target = reference_counter_register;
    load_instruction->type = HIRDataType::dword;
    load_instruction->address = reference_address;
    return reference_counter_register;
}

bonk::IRRegister bonk::HIRRefCountReplacer::adjust_reference_count(IRRegister reference_count,
                                                                   int64_t delta) {
    int constant_register = middle_end.id_table.get_unused_id();
    add_instruction<HIRConstantLoad>(constant_register, delta);

    int result_register = middle_end.id_table.get_unused_id();
    auto instruction = add_instruction<HIROperation>();
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
    auto store_instruction = add_instruction<HIRMemoryStore>();
    store_instruction->type = HIRDataType::dword;
    store_instruction->address = reference_address;
    store_instruction->value = value;
}

void bonk::HIRRefCountReplacer::call_destructor(TreeNodeHiveDefinition* hive_definition, IRRegister register_id) {
    std::string destructor_name =
        std::string(hive_definition->hive_name->identifier_text) + "$$destructor";

    // Find the destructor symbol
    auto scope = middle_end.symbol_table.get_scope_for_node(hive_definition)->parent_scope;
    auto destructor_definition = ScopedNameResolver(scope).get_name_definition(destructor_name);
    auto destructor_definition_id = middle_end.id_table.get_id(destructor_definition);

    auto destructor_call_parameter = add_instruction<HIRParameter>();
    destructor_call_parameter->type = HIRDataType::dword;
    destructor_call_parameter->parameter = register_id;

    auto destructor_call_instruction = add_instruction<HIRCall>();
    destructor_call_instruction->procedure_label_id = destructor_definition_id;
    destructor_call_instruction->return_type = HIRDataType::word;
}
