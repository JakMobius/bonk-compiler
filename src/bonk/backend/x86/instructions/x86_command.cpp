
#include "x86_command.hpp"
#include "../x86_backend_context.hpp"

namespace bonk::x86_backend {

const char* ASM_MNEMONICS[] = {"colorizer::dead_end",
                               "colorizer::scope_pop",
                               "colorizer::scope",
                               "colorizer::repeat_scope",
                               "colorizer::reg_preserve",
                               "colorizer::frame_create",
                               "colorizer::frame_destroy",
                               "colorizer::locate_register (in register)",
                               "colorizer::locate_register (in stack)",
                               "colorizer::align_stack (before)",
                               "colorizer::align_stack (after)",
                               "meta::jmp_label",
                               "mov",
                               "add",
                               "sub",
                               "imul",
                               "idiv",
                               "xor",
                               "push",
                               "pop",
                               "jmp",
                               "je",
                               "jne",
                               "jg",
                               "jng",
                               "jl",
                               "jge",
                               "sete",
                               "setne",
                               "setg",
                               "setng",
                               "setl",
                               "setge",
                               "xchg",
                               "ret",
                               "movzx",
                               "cmp",
                               "test",
                               "and",
                               "or",
                               "nop",
                               "call"};

const char* ASM_REGISTERS_64[] = {"rax", "rcx", "rdx", "rbx", "rsp", "rbp", "rsi", "rdi",
                                  "r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15"};

const char* ASM_REGISTERS_8[] = {"al",  "cl",  "dl",   "bl",   "spl",  "bpl",  "sil",  "dil",
                                 "r8b", "r9b", "r10b", "r11b", "r12b", "r13b", "r14b", "r15b"};

void asm_command::set_read_register(abstract_register reg) {
    read_registers.push_back(reg);
}

void asm_command::set_write_register(abstract_register reg) {
    write_registers.push_back(reg);
}

void asm_command::to_bytes(command_encoder* buffer) {
    assert(!"Command is not compilable");
}

command_parameter command_parameter::create_register_64(abstract_register reg) {
    command_parameter param(PARAMETER_TYPE_REG_64);
    param.reg = reg;
    return param;
}

command_parameter command_parameter::create_register_8(abstract_register reg) {
    command_parameter param(PARAMETER_TYPE_REG_8);
    param.reg = reg;
    return param;
}

command_parameter command_parameter::create_imm32(long long imm) {
    command_parameter param(PARAMETER_TYPE_IMM32);
    param.imm = imm;
    return param;
}

command_parameter command_parameter::create_label(jmp_label* label) {
    command_parameter param(PARAMETER_TYPE_LABEL);
    param.label = label;
    return param;
}

command_parameter command_parameter::create_memory(command_parameter_memory memory) {
    command_parameter param(PARAMETER_TYPE_MEMORY);
    param.memory = memory;
    return param;
}

command_parameter command_parameter::create_symbol(command_parameter_symbol symbol) {
    command_parameter param(PARAMETER_TYPE_SYMBOL);
    param.symbol = symbol;
    return param;
}

uint8_t command_parameter_memory::register_amount() {
    return (reg_a != abstract_register(-1)) + (reg_b != abstract_register(-1));
}

abstract_register command_parameter_memory::get_register(uint8_t index) {
    if (reg_a == abstract_register(-1))
        return reg_b;
    if (index == 0)
        return reg_a;
    return reg_b;
}

void command_parameter_memory::set_register(uint8_t index, abstract_register reg) {
    if (reg_a == abstract_register(-1))
        reg_b = reg;
    else if (index == 0)
        reg_a = reg;
    else
        reg_b = reg;
}

command_parameter_memory command_parameter_memory::create_reg(abstract_register reg) {
    return command_parameter_memory{
        .displacement = 0, .reg_a = reg, .reg_b = abstract_register(-1), .reg_a_constant = 1};
}

command_parameter_memory command_parameter_memory::create_reg_reg(abstract_register reg_a,
                                                                  abstract_register reg_b) {
    return command_parameter_memory{
        .displacement = 0, .reg_a = reg_a, .reg_b = reg_b, .reg_a_constant = 1};
}

command_parameter_memory command_parameter_memory::create_reg_displ(abstract_register reg,
                                                                    int32_t displacement) {
    return command_parameter_memory{.displacement = displacement,
                                    .reg_a = reg,
                                    .reg_b = abstract_register(-1),
                                    .reg_a_constant = 1};
}

command_parameter_memory command_parameter_memory::create_reg_reg_displ(abstract_register reg_a,
                                                                        abstract_register reg_b,
                                                                        int32_t displacement) {
    return command_parameter_memory{
        .displacement = displacement, .reg_a = reg_a, .reg_b = reg_b, .reg_a_constant = 1};
}

command_parameter_memory command_parameter_memory::create_reg_const_displ(abstract_register reg,
                                                                          uint8_t reg_a_constant,
                                                                          int32_t displacement) {
    return command_parameter_memory{.displacement = displacement,
                                    .reg_a = reg,
                                    .reg_b = abstract_register(-1),
                                    .reg_a_constant = reg_a_constant};
}

command_parameter_memory command_parameter_memory::create_reg_const_reg_displ(
    abstract_register reg_a, uint8_t reg_a_constant, abstract_register reg_b,
    int32_t displacement) {
    return command_parameter_memory{.displacement = displacement,
                                    .reg_a = reg_a,
                                    .reg_b = reg_b,
                                    .reg_a_constant = reg_a_constant};
}

command_parameter_memory command_parameter_memory::create_displ(int32_t displacement) {
    return command_parameter_memory{.displacement = displacement,
                                    .reg_a = abstract_register(-1),
                                    .reg_b = abstract_register(-1),
                                    .reg_a_constant = 1};
}

command_parameter_symbol::command_parameter_symbol(bool ip_relative, uint32_t symbol) {
    this->symbol = symbol;
    this->ip_relative = ip_relative;
}
} // namespace bonk::x86_backend