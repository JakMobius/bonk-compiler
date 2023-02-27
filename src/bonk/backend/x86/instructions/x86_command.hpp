#pragma once

namespace bonk::x86_backend {

struct AsmCommand;
struct CommandParameter;

enum AsmCommandType {
    COMMAND_COLORIZER_SCOPE_DEAD_END,
    COMMAND_COLORIZER_SCOPE_POP,
    COMMAND_COLORIZER_SCOPE,
    COMMAND_COLORIZER_REPEAT_SCOPE,
    COMMAND_COLORIZER_REG_PRESERVE,
    COMMAND_COLORIZER_FRAME_CREATE,
    COMMAND_COLORIZER_FRAME_DESTROY,
    COMMAND_COLORIZER_LOCATE_REG_REG,
    COMMAND_COLORIZER_LOCATE_REG_STACK,
    COMMAND_COLORIZER_ALIGN_STACK_BEFORE,
    COMMAND_COLORIZER_ALIGN_STACK_AFTER,
    COMMAND_JMP_LABEL,
    COMMAND_MOV,
    COMMAND_ADD,
    COMMAND_SUB,
    COMMAND_IMUL,
    COMMAND_IDIV,
    COMMAND_XOR,
    COMMAND_PUSH,
    COMMAND_POP,
    COMMAND_JMP,
    COMMAND_JE,
    COMMAND_JNE,
    COMMAND_JG,
    COMMAND_JNG,
    COMMAND_JL,
    COMMAND_JNL,
    COMMAND_SETE,
    COMMAND_SETNE,
    COMMAND_SETG,
    COMMAND_SETNG,
    COMMAND_SETL,
    COMMAND_SETNL,
    COMMAND_XCHG,
    COMMAND_RET,
    COMMAND_MOVZX,
    COMMAND_CMP,
    COMMAND_TEST,
    COMMAND_AND,
    COMMAND_OR,
    COMMAND_NOP,
    COMMAND_CALL
};

extern const char* ASM_MNEMONICS[];
extern const char* ASM_REGISTERS_64[];
extern const char* ASM_REGISTERS_8[];

} // namespace bonk::x86_backend

#include "../../../../utils/linear_allocator.hpp"
#include "../x86_abstract_register.hpp"
#include "../x86_command_buffer.hpp"
#include "../x86_command_encoder.hpp"
#include "../x86_register_extensions.hpp"

namespace bonk::x86_backend {

struct BackendContext;

struct CommandParameterMemory {
    int32_t displacement;
    AbstractRegister reg_a;
    AbstractRegister reg_b;
    uint8_t reg_a_constant;

    uint8_t register_amount() const;

    AbstractRegister get_register(uint8_t index) const;

    void set_register(uint8_t index, AbstractRegister reg);

    static CommandParameterMemory create_reg(AbstractRegister reg);

    static CommandParameterMemory create_displ(int32_t displacement);

    static CommandParameterMemory create_reg_displ(AbstractRegister reg, int32_t displacement);

    static CommandParameterMemory create_reg_reg(AbstractRegister reg_a,
                                                   AbstractRegister reg_b);

    static CommandParameterMemory
    create_reg_reg_displ(AbstractRegister reg_a, AbstractRegister reg_b, int32_t displacement);

    static CommandParameterMemory
    create_reg_const_displ(AbstractRegister reg, uint8_t reg_a_constant, int32_t displacement);

    static CommandParameterMemory create_reg_const_reg_displ(AbstractRegister reg_a,
                                                               uint8_t reg_a_constant,
                                                               AbstractRegister reg_b,
                                                               int32_t displacement);
};

struct CommandParameterSymbol {
    uint32_t symbol;
    bool ip_relative;

    CommandParameterSymbol(bool ip_relative, uint32_t symbol);

    CommandParameterSymbol() : symbol(0), ip_relative(false) {
    }
};

enum CommandParameterType {
    PARAMETER_TYPE_UNSET,
    PARAMETER_TYPE_REG_64,
    PARAMETER_TYPE_REG_8,
    PARAMETER_TYPE_MEMORY,
    PARAMETER_TYPE_IMM32,
    PARAMETER_TYPE_LABEL,
    PARAMETER_TYPE_SYMBOL
};

struct CommandParameter {
    union {
        long long imm;
        AbstractRegister reg;
        JmpLabel* label;
        CommandParameterMemory memory;
        CommandParameterSymbol symbol;
    };
    CommandParameterType type;

    CommandParameter() : type(PARAMETER_TYPE_UNSET){};

    CommandParameter(CommandParameterType type) : type(type){};

    static CommandParameter create_register_64(AbstractRegister reg);

    static CommandParameter create_register_8(AbstractRegister reg);

    static CommandParameter create_imm32(long long imm);

    static CommandParameter create_label(JmpLabel* label);

    static CommandParameter create_memory(CommandParameterMemory memory);

    static CommandParameter create_symbol(CommandParameterSymbol symbol);
};

struct AsmCommand {
    AsmCommandType type;
    long offset;

    std::vector<AbstractRegister> read_registers;
    std::vector<AbstractRegister> write_registers;
    std::vector<CommandParameter> parameters;

    void set_read_register(AbstractRegister reg);

    void set_write_register(AbstractRegister reg);

    virtual void to_bytes(CommandEncoder* buffer);

    virtual AsmCommand* clone() = 0;

  protected:
    void copy_parameters(AsmCommand* other);
};

}; // namespace bonk::x86_backend
