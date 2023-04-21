
#include "hir.hpp"
#include <ostream>

bonk::HIRInstruction::HIRInstruction(bonk::HIRInstructionType type) : type(type) {
}

void bonk::HIRProgram::create_procedure() {
    procedures.push_back(std::make_unique<HIRProcedure>(*this));
}

void bonk::HIRProcedure::add_control_flow_edge(bonk::HIRBaseBlock* from, bonk::HIRBaseBlock* to) {
    from->successors.push_back(to);
    to->predecessors.push_back(from);
}

void bonk::HIRProcedure::create_base_block() {
    auto block = std::make_unique<HIRBaseBlock>(*this);
    block->index = base_blocks.size();
    base_blocks.push_back(std::move(block));
}
int bonk::HIRProcedure::get_unused_register() {
    return used_registers++;
}

void bonk::HIRProcedure::rename_blocks(int* old_to_new) {

    // Relink the jmp and jnz instructions and start/end block indices

    assert(old_to_new[start_block_index] != -1);
    assert(old_to_new[end_block_index] != -1);

    start_block_index = old_to_new[start_block_index];
    end_block_index = old_to_new[end_block_index];

    for (auto& block : base_blocks) {
        if (block->instructions.empty()) {
            continue;
        }
        auto& last_instruction = block->instructions.back();
        if (last_instruction->type == HIRInstructionType::jump) {
            auto* jmp = (HIRJumpInstruction*)last_instruction;
            jmp->label_id = old_to_new[jmp->label_id];

            assert(jmp->label_id != -1);
        } else if (last_instruction->type == HIRInstructionType::jump_nz) {
            auto* jnz = (HIRJumpNZInstruction*)last_instruction;
            jnz->z_label = old_to_new[jnz->z_label];
            jnz->nz_label = old_to_new[jnz->nz_label];

            assert(jnz->z_label != -1);
            assert(jnz->nz_label != -1);
        }
    }

    int new_block_count = 0;

    for (int i = 0; i < base_blocks.size(); i++) {
        if (old_to_new[i] != -1) {
            new_block_count++;
        }
    }

    int unused_index = new_block_count;

    for (int i = 0; i < base_blocks.size(); i++) {
        if (old_to_new[i] == -1) {
            old_to_new[i] = unused_index++;
        }
    }

    for (int i = 0; i < base_blocks.size(); i++) {
        while (old_to_new[i] != i) {
            std::swap(base_blocks[i], base_blocks[old_to_new[i]]);
            std::swap(old_to_new[i], old_to_new[old_to_new[i]]);
        }
        base_blocks[i]->index = i;
    }

    base_blocks.resize(new_block_count);
}

void bonk::HIRProcedure::remove_killed_blocks() {
    std::vector<int> old_to_new(base_blocks.size(), -1);

    int new_block_count = 0;

    for (int i = 0; new_block_count < base_blocks.size(); new_block_count++) {
        while (i < base_blocks.size() && base_blocks[i]->index == -1) {
            i++;
        }
        if (i < base_blocks.size()) {
            base_blocks[i]->remove_killed_edges();
            old_to_new[i++] = new_block_count;
        } else {
            break;
        }
    }

    rename_blocks(old_to_new.data());
}
void bonk::HIRProcedure::remove_control_flow_edge(bonk::HIRBaseBlock* from,
                                                  bonk::HIRBaseBlock* to) {
    auto from_succ = std::find(from->successors.begin(), from->successors.end(), to);
    auto to_pred = std::find(to->predecessors.begin(), to->predecessors.end(), from);

    assert(from_succ != from->successors.end());
    assert(to_pred != to->predecessors.end());

    int to_pred_index = to_pred - to->predecessors.begin();

    from->successors.erase(from_succ);
    to->predecessors.erase(to_pred);

    // If the 'to' block has phi instructions, we need to remove the 'from' block from them

    for (auto& instruction : to->instructions) {
        if (instruction->type != HIRInstructionType::phi_function) {
            break;
        }
        auto* phi = (HIRPhiFunctionInstruction*)instruction;
        phi->sources.erase(phi->sources.begin() + to_pred_index);
    }
}

void bonk::HIRBaseBlock::remove_killed_edges() {
    int new_index = 0;
    for (auto& successor : successors) {
        if (successor->index != -1) {
            successors[new_index++] = successor;
        }
    }
    successors.resize(new_index);

    new_index = 0;
    for (auto& predecessor : predecessors) {
        if (predecessor->index != -1) {
            predecessors[new_index++] = predecessor;
        }
    }
    predecessors.resize(new_index);
}

bonk::HIRLabelInstruction::HIRLabelInstruction(int label_id)
    : HIRInstruction(HIRInstructionType::label), label_id(label_id) {
}

bonk::HIRConstantLoadInstruction::HIRConstantLoadInstruction(bonk::IRRegister target,
                                                             long long int constant,
                                                             bonk::HIRDataType type)
    : HIRInstruction(HIRInstructionType::constant_load), target(target), type(type),
      constant(constant) {
}

bonk::HIRConstantLoadInstruction::HIRConstantLoadInstruction(bonk::IRRegister target,
                                                             int64_t constant)
    : HIRInstruction(HIRInstructionType::constant_load), target(target), type(HIRDataType::dword),
      constant(constant) {
}

bonk::HIRConstantLoadInstruction::HIRConstantLoadInstruction(bonk::IRRegister target,
                                                             int32_t constant)
    : HIRInstruction(HIRInstructionType::constant_load), target(target), type(HIRDataType::word),
      constant(constant) {
}

bonk::HIRConstantLoadInstruction::HIRConstantLoadInstruction(bonk::IRRegister target,
                                                             int16_t constant)
    : HIRInstruction(HIRInstructionType::constant_load), target(target), type(HIRDataType::hword),
      constant(constant) {
}

bonk::HIRConstantLoadInstruction::HIRConstantLoadInstruction(bonk::IRRegister target,
                                                             int8_t constant)
    : HIRInstruction(HIRInstructionType::constant_load), target(target), type(HIRDataType::byte),
      constant(constant) {
}

bonk::HIRConstantLoadInstruction::HIRConstantLoadInstruction(bonk::IRRegister target,
                                                             float constant)
    : HIRInstruction(HIRInstructionType::constant_load), target(target), type(HIRDataType::float32),
      constant(*reinterpret_cast<uint32_t*>(&constant)) {
}

bonk::HIRConstantLoadInstruction::HIRConstantLoadInstruction(bonk::IRRegister target,
                                                             double constant)
    : HIRInstruction(HIRInstructionType::constant_load), target(target), type(HIRDataType::float64),
      constant(*reinterpret_cast<uint64_t*>(&constant)) {
}

bonk::HIRSymbolLoadInstruction::HIRSymbolLoadInstruction(bonk::IRRegister target, int symbol_id,
                                                         bonk::HIRDataType type)
    : HIRInstruction(HIRInstructionType::symbol_load), target(target), type(type),
      symbol_id(symbol_id) {
}

bonk::HIROperationInstruction::HIROperationInstruction()
    : HIRInstruction(HIRInstructionType::operation) {
}

bonk::HIROperationInstruction& bonk::HIROperationInstruction::set_assign(bonk::IRRegister target,
                                                                         bonk::IRRegister left,
                                                                         bonk::HIRDataType type) {
    this->target = target;
    this->left = left;
    this->right = std::nullopt;
    this->operation_type = HIROperationType::assign;
    this->operand_type = type;
    this->result_type = type;
    return *this;
}

bonk::HIRJumpInstruction::HIRJumpInstruction()
    : HIRInstruction(HIRInstructionType::jump), label_id() {
}

bonk::HIRJumpInstruction::HIRJumpInstruction(int label)
    : HIRInstruction(HIRInstructionType::jump), label_id(label) {
}

bonk::HIRJumpNZInstruction::HIRJumpNZInstruction() : HIRInstruction(HIRInstructionType::jump_nz) {
}

bonk::HIRJumpNZInstruction::HIRJumpNZInstruction(bonk::IRRegister condition, int nz_label,
                                                 int z_label)
    : HIRInstruction(HIRInstructionType::jump_nz), condition(condition), nz_label(nz_label),
      z_label(z_label) {
}

bonk::HIRCallInstruction::HIRCallInstruction()
    : HIRInstruction(HIRInstructionType::call), procedure_label_id() {
}

bonk::HIRReturnInstruction::HIRReturnInstruction(bonk::IRRegister return_value)
    : HIRInstruction(HIRInstructionType::return_op), return_value(return_value) {
}

bonk::HIRReturnInstruction::HIRReturnInstruction() : HIRInstruction(HIRInstructionType::return_op) {
}

bonk::HIRParameterInstruction::HIRParameterInstruction()
    : HIRInstruction(HIRInstructionType::parameter) {
}

bonk::HIRMemoryLoadInstruction::HIRMemoryLoadInstruction()
    : HIRInstruction(HIRInstructionType::memory_load) {
}

bonk::HIRMemoryLoadInstruction::HIRMemoryLoadInstruction(IRRegister target, IRRegister address,
                                                         HIRDataType type)
    : HIRInstruction(HIRInstructionType::memory_load), target(target), address(address),
      type(type) {
}

bonk::HIRMemoryStoreInstruction::HIRMemoryStoreInstruction()
    : HIRInstruction(HIRInstructionType::memory_store) {
}

bonk::HIRIncRefCounterInstruction::HIRIncRefCounterInstruction()
    : HIRInstruction(HIRInstructionType::inc_ref_counter) {
}

bonk::HIRDecRefCounterInstruction::HIRDecRefCounterInstruction()
    : HIRInstruction(HIRInstructionType::dec_ref_counter) {
}

bonk::HIRFileInstruction::HIRFileInstruction() : HIRInstruction(HIRInstructionType::file) {
}

bonk::HIRLocationInstruction::HIRLocationInstruction()
    : HIRInstruction(HIRInstructionType::location) {
}

bonk::HIRPhiFunctionInstruction::HIRPhiFunctionInstruction()
    : HIRInstruction(HIRInstructionType::phi_function) {
}
