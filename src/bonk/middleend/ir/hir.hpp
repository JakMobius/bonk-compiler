#pragma once

namespace bonk {

enum class HIRInstructionType {
    unset,
    label,
    constant_load,
    symbol_load,
    operation,
    jump,
    jump_nz,
    call,
    return_op,
    parameter,
    procedure,
    memory_load,
    memory_store,
};

enum class HIROperationType {
    plus,
    minus,
    multiply,
    divide,
    assign,
    and_op,
    or_op,
    xor_op,
    not_op,
    equal,
    not_equal,
    less,
    less_equal,
    greater,
    greater_equal
};

enum class HIRDataType {
    unset,
    byte,
    hword,
    word,
    dword,
    float32,
    float64,
};

} // namespace bonk

#include <optional>
#include "ir.hpp"

namespace bonk {

struct HIRInstruction : IRInstruction {
    HIRInstructionType type = HIRInstructionType::unset;

    HIRInstruction(HIRInstructionType type);
    virtual ~HIRInstruction() = default;
};

struct HIRLabel : HIRInstruction {
    int label_id = -1;

    HIRLabel(int label_id);
};

struct HIRConstantLoad : HIRInstruction {
    IRRegister target = 0;
    HIRDataType type = HIRDataType::unset;
    unsigned long long constant = 0;

    HIRConstantLoad(IRRegister target, unsigned long long constant, HIRDataType type);
    HIRConstantLoad(IRRegister target, uint64_t constant);
    HIRConstantLoad(IRRegister target, uint32_t constant);
    HIRConstantLoad(IRRegister target, uint16_t constant);
    HIRConstantLoad(IRRegister target, uint8_t constant);
    HIRConstantLoad(IRRegister target, float constant);
    HIRConstantLoad(IRRegister target, double constant);
};

struct HIRSymbolLoad : HIRInstruction {
    IRRegister target = 0;
    HIRDataType type = HIRDataType::unset;
    int symbol_id = -1;

    HIRSymbolLoad(IRRegister target, int symbol_id, HIRDataType type);
};

struct HIROperation : HIRInstruction {
    IRRegister target{};
    IRRegister left{};
    std::optional<IRRegister> right{};
    HIROperationType operation_type = HIROperationType::plus;
    HIRDataType operand_type = HIRDataType::unset;
    HIRDataType result_type = HIRDataType::unset;

    HIROperation();
};

struct HIRJump : HIRInstruction {
    int label_id = -1;

    HIRJump();
};

struct HIRJumpNZ : HIRInstruction {
    IRRegister condition = 0;
    int nz_label = -1;
    int z_label = -1;

    HIRJumpNZ(IRRegister condition, int nz_label, int z_label);
};

struct HIRCall : HIRInstruction {
    HIRDataType return_type = HIRDataType::unset;
    std::optional<IRRegister> return_value = std::nullopt;

    int procedure_label_id = -1;

    HIRCall();
};

struct HIRReturn : HIRInstruction {
    HIRDataType return_type = HIRDataType::unset;
    std::optional<IRRegister> return_value = std::nullopt;

    HIRReturn(IRRegister return_value);
    HIRReturn();
};

struct HIRParameter : HIRInstruction {
    HIRDataType type = HIRDataType::unset;
    IRRegister parameter = 0;

    HIRParameter();
};

struct HIRProcedureParameter {
    HIRDataType type = HIRDataType::unset;
    IRRegister register_id = 0;
};

struct HIRProcedure : HIRInstruction {
    int procedure_id = -1;
    std::vector<HIRProcedureParameter> parameters;
    HIRDataType return_type = HIRDataType::unset;
    bool is_external = false;

    HIRProcedure(int procedure_id, HIRDataType return_type);
};

struct HIRMemoryLoad : HIRInstruction {
    IRRegister target = 0;
    IRRegister address = 0;
    HIRDataType type = HIRDataType::unset;

    HIRMemoryLoad();
};

struct HIRMemoryStore : HIRInstruction {
    IRRegister address = 0;
    IRRegister value = 0;
    HIRDataType type = HIRDataType::unset;

    HIRMemoryStore();
};

} // namespace bonk