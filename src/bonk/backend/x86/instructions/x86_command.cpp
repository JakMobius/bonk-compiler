
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

void AsmCommand::set_read_register(AbstractRegister reg) {
    read_registers.push_back(reg);
}

void AsmCommand::set_write_register(AbstractRegister reg) {
    write_registers.push_back(reg);
}

void AsmCommand::to_bytes(CommandEncoder* buffer) {
    assert(!"Command is not compilable");
}

CommandParameter CommandParameter::create_register_64(AbstractRegister reg) {
    CommandParameter param(PARAMETER_TYPE_REG_64);
    param.reg = reg;
    return param;
}

CommandParameter CommandParameter::create_register_8(AbstractRegister reg) {
    CommandParameter param(PARAMETER_TYPE_REG_8);
    param.reg = reg;
    return param;
}

CommandParameter CommandParameter::create_imm32(long long imm) {
    CommandParameter param(PARAMETER_TYPE_IMM32);
    param.imm = imm;
    return param;
}

CommandParameter CommandParameter::create_label(JmpLabel* label) {
    CommandParameter param(PARAMETER_TYPE_LABEL);
    param.label = label;
    return param;
}

CommandParameter CommandParameter::create_memory(CommandParameterMemory memory) {
    CommandParameter param(PARAMETER_TYPE_MEMORY);
    param.memory = memory;
    return param;
}

CommandParameter CommandParameter::create_symbol(CommandParameterSymbol symbol) {
    CommandParameter param(PARAMETER_TYPE_SYMBOL);
    param.symbol = symbol;
    return param;
}

uint8_t CommandParameterMemory::register_amount() {
    return (reg_a != AbstractRegister(-1)) + (reg_b != AbstractRegister(-1));
}

AbstractRegister CommandParameterMemory::get_register(uint8_t index) {
    if (reg_a == AbstractRegister(-1))
        return reg_b;
    if (index == 0)
        return reg_a;
    return reg_b;
}

void CommandParameterMemory::set_register(uint8_t index, AbstractRegister reg) {
    if (reg_a == AbstractRegister(-1))
        reg_b = reg;
    else if (index == 0)
        reg_a = reg;
    else
        reg_b = reg;
}

CommandParameterMemory CommandParameterMemory::create_reg(AbstractRegister reg) {
    return CommandParameterMemory{
        .displacement = 0, .reg_a = reg, .reg_b = AbstractRegister(-1), .reg_a_constant = 1};
}

CommandParameterMemory CommandParameterMemory::create_reg_reg(AbstractRegister reg_a,
                                                              AbstractRegister reg_b) {
    return CommandParameterMemory{
        .displacement = 0, .reg_a = reg_a, .reg_b = reg_b, .reg_a_constant = 1};
}

CommandParameterMemory CommandParameterMemory::create_reg_displ(AbstractRegister reg,
                                                                    int32_t displacement) {
    return CommandParameterMemory{.displacement = displacement,
                                    .reg_a = reg,
                                    .reg_b = AbstractRegister(-1),
                                    .reg_a_constant = 1};
}

CommandParameterMemory CommandParameterMemory::create_reg_reg_displ(AbstractRegister reg_a,
                                                                    AbstractRegister reg_b,
                                                                        int32_t displacement) {
    return CommandParameterMemory{
        .displacement = displacement, .reg_a = reg_a, .reg_b = reg_b, .reg_a_constant = 1};
}

CommandParameterMemory CommandParameterMemory::create_reg_const_displ(AbstractRegister reg,
                                                                          uint8_t reg_a_constant,
                                                                          int32_t displacement) {
    return CommandParameterMemory{.displacement = displacement,
                                    .reg_a = reg,
                                    .reg_b = AbstractRegister(-1),
                                    .reg_a_constant = reg_a_constant};
}

CommandParameterMemory CommandParameterMemory::create_reg_const_reg_displ(AbstractRegister reg_a, uint8_t reg_a_constant,
                                                                          AbstractRegister reg_b,
    int32_t displacement) {
    return CommandParameterMemory{.displacement = displacement,
                                    .reg_a = reg_a,
                                    .reg_b = reg_b,
                                    .reg_a_constant = reg_a_constant};
}

CommandParameterMemory CommandParameterMemory::create_displ(int32_t displacement) {
    return CommandParameterMemory{.displacement = displacement,
                                    .reg_a = AbstractRegister(-1),
                                    .reg_b = AbstractRegister(-1),
                                    .reg_a_constant = 1};
}

CommandParameterSymbol::CommandParameterSymbol(bool ip_relative, uint32_t symbol) {
    this->symbol = symbol;
    this->ip_relative = ip_relative;
}
} // namespace bonk::x86_backend