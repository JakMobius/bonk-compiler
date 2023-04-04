
#include "hir.hpp"
#include <ostream>

bonk::HIRInstruction::HIRInstruction(bonk::HIRInstructionType type) : type(type) {
}

bonk::HIRLabel::HIRLabel(int label_id)
    : HIRInstruction(HIRInstructionType::label), label_id(label_id) {
}

bonk::HIRConstantLoad::HIRConstantLoad(bonk::IRRegister target, long long int constant,
                                       bonk::HIRDataType type)
    : HIRInstruction(HIRInstructionType::constant_load), target(target), type(type),
      constant(constant) {
}

bonk::HIRConstantLoad::HIRConstantLoad(bonk::IRRegister target, int64_t constant)
    : HIRInstruction(HIRInstructionType::constant_load), target(target), type(HIRDataType::dword),
      constant(constant) {
}

bonk::HIRConstantLoad::HIRConstantLoad(bonk::IRRegister target, int32_t constant)
    : HIRInstruction(HIRInstructionType::constant_load), target(target), type(HIRDataType::word),
      constant(constant) {
}

bonk::HIRConstantLoad::HIRConstantLoad(bonk::IRRegister target, int16_t constant)
    : HIRInstruction(HIRInstructionType::constant_load), target(target), type(HIRDataType::hword),
      constant(constant) {
}

bonk::HIRConstantLoad::HIRConstantLoad(bonk::IRRegister target, int8_t constant)
    : HIRInstruction(HIRInstructionType::constant_load), target(target), type(HIRDataType::byte),
      constant(constant) {
}

bonk::HIRConstantLoad::HIRConstantLoad(bonk::IRRegister target, float constant)
    : HIRInstruction(HIRInstructionType::constant_load), target(target), type(HIRDataType::float32),
      constant(*reinterpret_cast<uint32_t*>(&constant)) {
}

bonk::HIRConstantLoad::HIRConstantLoad(bonk::IRRegister target, double constant)
    : HIRInstruction(HIRInstructionType::constant_load), target(target), type(HIRDataType::float64),
      constant(*reinterpret_cast<uint64_t*>(&constant)) {
}

bonk::HIRSymbolLoad::HIRSymbolLoad(bonk::IRRegister target, int symbol_id, bonk::HIRDataType type)
    : HIRInstruction(HIRInstructionType::symbol_load), target(target), type(type),
      symbol_id(symbol_id) {
}

bonk::HIROperation::HIROperation() : HIRInstruction(HIRInstructionType::operation) {
}

bonk::HIRJump::HIRJump() : HIRInstruction(HIRInstructionType::jump), label_id() {
}

bonk::HIRJumpNZ::HIRJumpNZ(bonk::IRRegister condition, int nz_label, int z_label)
    : HIRInstruction(HIRInstructionType::jump_nz), condition(condition), nz_label(nz_label),
      z_label(z_label) {
}

bonk::HIRCall::HIRCall() : HIRInstruction(HIRInstructionType::call), procedure_label_id() {
}

bonk::HIRReturn::HIRReturn(bonk::IRRegister return_value)
    : HIRInstruction(HIRInstructionType::return_op), return_value(return_value) {
}

bonk::HIRReturn::HIRReturn() : HIRInstruction(HIRInstructionType::return_op) {
}

bonk::HIRParameter::HIRParameter() : HIRInstruction(HIRInstructionType::parameter) {
}

bonk::HIRProcedure::HIRProcedure(int procedure_id, bonk::HIRDataType return_type)
    : HIRInstruction(HIRInstructionType::procedure), procedure_id(procedure_id),
      return_type(return_type) {
}

bonk::HIRMemoryLoad::HIRMemoryLoad() : HIRInstruction(HIRInstructionType::memory_load) {
}

bonk::HIRMemoryStore::HIRMemoryStore() : HIRInstruction(HIRInstructionType::memory_store) {
}

bonk::HIRIncRefCounter::HIRIncRefCounter() : HIRInstruction(HIRInstructionType::inc_ref_counter) {
}

bonk::HIRDecRefCounter::HIRDecRefCounter() : HIRInstruction(HIRInstructionType::dec_ref_counter) {
}

bonk::HIRFile::HIRFile() : HIRInstruction(HIRInstructionType::file) {
}

bonk::HIRLocation::HIRLocation() : HIRInstruction(HIRInstructionType::location) {
}
