
#include "x86_mov.hpp"

namespace bonk::x86_backend {

mov_command::mov_command(command_parameter parameter1, command_parameter parameter2) {
    type = COMMAND_MOV;

    parameters.push_back(parameter1);
    parameters.push_back(parameter2);

    if (parameter2.type == PARAMETER_TYPE_REG_64 || parameter2.type == PARAMETER_TYPE_REG_8) {
        set_read_register(parameter2.reg);
    }

    if (parameter1.type == PARAMETER_TYPE_REG_64 || parameter1.type == PARAMETER_TYPE_REG_8) {
        set_write_register(parameter1.reg);
    }

    if (parameter1.type == PARAMETER_TYPE_MEMORY) {
        for (int i = parameter1.memory.register_amount() - 1; i >= 0; i--) {
            set_read_register(parameter1.memory.get_register(i));
        }
    }

    if (parameter2.type == PARAMETER_TYPE_MEMORY) {
        for (int i = parameter2.memory.register_amount() - 1; i >= 0; i--) {
            set_read_register(parameter2.memory.get_register(i));
        }
    }
}

void mov_command::to_bytes(command_encoder* buffer) {

    command_parameter target = parameters[0];
    command_parameter source = parameters[1];

    if (target.type == PARAMETER_TYPE_REG_64 && source.type == PARAMETER_TYPE_REG_64) {
        buffer->write_prefix_opcode_modrm_sib(0x89, source, target);

        return;
    }

    if (target.type == PARAMETER_TYPE_REG_64 && source.type == PARAMETER_TYPE_IMM32) {
        register_extensions rex = {.w = true, .b = target.reg > 7};

        buffer->buffer.push_back(rex.get_byte());
        buffer->buffer.push_back(0xb8 + (target.reg & 0b111)); // movabs opcode
        for (int i = 0; i < sizeof(uint64_t); i++)
            buffer->buffer.push_back(((char*)&source.imm)[i]);

        return;
    }

    if (target.type == PARAMETER_TYPE_REG_64 && source.type == PARAMETER_TYPE_MEMORY) {
        buffer->write_prefix_opcode_modrm_sib(0x8B, target, source);

        return;
    }

    if (target.type == PARAMETER_TYPE_MEMORY && source.type == PARAMETER_TYPE_REG_64) {
        buffer->write_prefix_opcode_modrm_sib(0x89, source, target);

        return;
    }

    if (target.type == PARAMETER_TYPE_REG_8 && source.type == PARAMETER_TYPE_IMM32) {
        register_extensions rex = {.b = target.reg > 7};

        buffer->buffer.push_back(rex.get_byte());
        buffer->buffer.push_back(0xb0 + (target.reg & 0b111)); // mov r8 imm8 opcode
        for (int i = 0; i < sizeof(uint8_t); i++)
            buffer->buffer.push_back(((char*)&source.imm)[i]);

        return;
    }

    if (target.type == PARAMETER_TYPE_REG_64 && source.type == PARAMETER_TYPE_SYMBOL) {
        buffer->write_prefix_opcode_modrm_sib(0x8B, target, source);

        return;
    }

    if (target.type == PARAMETER_TYPE_SYMBOL && source.type == PARAMETER_TYPE_REG_64) {
        buffer->write_prefix_opcode_modrm_sib(0x89, source, target);

        return;
    }

    assert(!"Cannot encode command");
}

void movzx_command::to_bytes(command_encoder* buffer) {

    parameters.resize(2);
    command_parameter target = parameters[0];
    command_parameter source = parameters[1];

    if (target.type == PARAMETER_TYPE_REG_64 && source.type == PARAMETER_TYPE_REG_8) {
        buffer->write_prefix_longopcode_regrm_sib(0x0F, 0xB6, source, target);

        return;
    }

    assert(!"Cannot encode command");
}

} // namespace bonk::x86_backend