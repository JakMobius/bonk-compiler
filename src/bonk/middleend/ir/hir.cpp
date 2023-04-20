
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

bonk::HIRPhiFunctionInstruction::HIRPhiFunctionInstruction(): HIRInstruction(HIRInstructionType::phi_function){
}
