#pragma once

namespace bonk {

enum class HIRInstructionType {
    label,
    constant_load,
    symbol_load,
    operation,
    jump,
    jump_nz,
    call,
    return_op,
    parameter,
    procedure
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
    HIRInstructionType type;

    HIRInstruction(HIRInstructionType type);
    virtual ~HIRInstruction() = default;
};

struct HIRLabel : HIRInstruction {
    int label_id;

    HIRLabel(int label_id);
};

struct HIRConstantLoad : HIRInstruction {
    IRRegister target;
    HIRDataType type;
    unsigned long long constant;

    HIRConstantLoad(IRRegister target, unsigned long long constant, HIRDataType type);
    HIRConstantLoad(IRRegister target, uint64_t constant);
    HIRConstantLoad(IRRegister target, uint32_t constant);
    HIRConstantLoad(IRRegister target, uint16_t constant);
    HIRConstantLoad(IRRegister target, uint8_t constant);
    HIRConstantLoad(IRRegister target, float constant);
    HIRConstantLoad(IRRegister target, double constant);
};

struct HIRSymbolLoad : HIRInstruction {
    IRRegister target;
    HIRDataType type;
    int symbol_id;

    HIRSymbolLoad(IRRegister target, int symbol_id, HIRDataType type);
};

struct HIROperation : HIRInstruction {
    IRRegister target{};
    IRRegister left{};
    std::optional<IRRegister> right{};
    HIROperationType operation_type;
    HIRDataType operand_type;
    HIRDataType result_type;

    HIROperation();
};

struct HIRJump : HIRInstruction {
    int label_id = -1;

    HIRJump();
};

struct HIRJumpNZ : HIRInstruction {
    IRRegister condition;
    int nz_label;
    int z_label;

    HIRJumpNZ(IRRegister condition, int nz_label, int z_label);
};

struct HIRCall : HIRInstruction {
    HIRDataType return_type;
    std::optional<IRRegister> return_value;

    int procedure_label_id = -1;

    HIRCall();
};

struct HIRReturn : HIRInstruction {
    HIRDataType return_type;
    std::optional<IRRegister> return_value;

    HIRReturn(IRRegister return_value);
    HIRReturn();
};

struct HIRParameter : HIRInstruction {
    HIRDataType type;
    IRRegister parameter;

    HIRParameter();
};

struct HIRProcedureParameter {
    HIRDataType type;
    IRRegister register_id;
};

struct HIRProcedure : HIRInstruction {
    int procedure_id;
    std::vector<HIRProcedureParameter> parameters;
    HIRDataType return_type;

    HIRProcedure(int procedure_id, HIRDataType return_type);
};

} // namespace bonk