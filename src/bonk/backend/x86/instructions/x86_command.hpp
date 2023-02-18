#pragma once

namespace bonk::x86_backend {

struct asm_command;
struct command_parameter;

enum asm_command_type {
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

struct backend_context;

struct command_parameter_memory {
    int32_t displacement;
    abstract_register reg_a;
    abstract_register reg_b;
    uint8_t reg_a_constant;

    uint8_t register_amount();

    abstract_register get_register(uint8_t index);

    void set_register(uint8_t index, abstract_register reg);

    static command_parameter_memory create_reg(abstract_register reg);

    static command_parameter_memory create_displ(int32_t displacement);

    static command_parameter_memory create_reg_displ(abstract_register reg, int32_t displacement);

    static command_parameter_memory create_reg_reg(abstract_register reg_a,
                                                   abstract_register reg_b);

    static command_parameter_memory
    create_reg_reg_displ(abstract_register reg_a, abstract_register reg_b, int32_t displacement);

    static command_parameter_memory
    create_reg_const_displ(abstract_register reg, uint8_t reg_a_constant, int32_t displacement);

    static command_parameter_memory create_reg_const_reg_displ(abstract_register reg_a,
                                                               uint8_t reg_a_constant,
                                                               abstract_register reg_b,
                                                               int32_t displacement);
};

struct command_parameter_symbol {
    uint32_t symbol;
    bool ip_relative;

    command_parameter_symbol(bool ip_relative, uint32_t symbol);

    command_parameter_symbol() : symbol(0), ip_relative(false) {
    }
};

enum command_parameter_type {
    PARAMETER_TYPE_UNSET,
    PARAMETER_TYPE_REG_64,
    PARAMETER_TYPE_REG_8,
    PARAMETER_TYPE_MEMORY,
    PARAMETER_TYPE_IMM32,
    PARAMETER_TYPE_LABEL,
    PARAMETER_TYPE_SYMBOL
};

struct command_parameter {
    union {
        long long imm;
        abstract_register reg;
        jmp_label* label;
        command_parameter_memory memory;
        command_parameter_symbol symbol;
    };
    command_parameter_type type;

    command_parameter() : type(PARAMETER_TYPE_UNSET){};

    command_parameter(command_parameter_type type) : type(type){};

    static command_parameter create_register_64(abstract_register reg);

    static command_parameter create_register_8(abstract_register reg);

    static command_parameter create_imm32(long long imm);

    static command_parameter create_label(jmp_label* label);

    static command_parameter create_memory(command_parameter_memory memory);

    static command_parameter create_symbol(command_parameter_symbol symbol);
};

struct asm_command {
    asm_command_type type;
    long offset;

    std::vector<abstract_register> read_registers;
    std::vector<abstract_register> write_registers;
    std::vector<command_parameter> parameters;

    void set_read_register(abstract_register reg);

    void set_write_register(abstract_register reg);

    virtual void to_bytes(command_encoder* buffer);

    virtual asm_command* clone() = 0;

  protected:
    void copy_parameters(asm_command* other);
};

}; // namespace bonk::x86_backend
