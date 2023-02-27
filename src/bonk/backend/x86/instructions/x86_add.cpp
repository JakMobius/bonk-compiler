
#include "x86_add.hpp"

namespace bonk::x86_backend {

AddCommand::AddCommand(CommandParameter target, CommandParameter source) {
    type = COMMAND_ADD;

    assert(target.type == PARAMETER_TYPE_REG_64 || target.type == PARAMETER_TYPE_REG_8);

    parameters.resize(2);
    parameters[0] = target;
    parameters[1] = source;

    if (source.type == PARAMETER_TYPE_REG_64 || source.type == PARAMETER_TYPE_REG_8) {
        assert(source.type == target.type);
        set_read_register(source.reg);
    } else {
        assert(source.type == PARAMETER_TYPE_IMM32);
    }

    set_read_register(target.reg);
    set_write_register(target.reg);
}

void AddCommand::to_bytes(CommandEncoder* buffer) {

    CommandParameter target = parameters[0];
    CommandParameter source = parameters[1];

    if (target.type == PARAMETER_TYPE_REG_64 && source.type == PARAMETER_TYPE_REG_64) {
        // Turned out that these arguments must be passed in reverse order.
        buffer->write_prefix_opcode_modrm_sib(0x01, source, target);

        return;
    }

    if (target.type == PARAMETER_TYPE_REG_64 && source.type == PARAMETER_TYPE_IMM32) {
        buffer->write_extended_opcode(0x81, 0x00, target);
        for (int i = 0; i < sizeof(uint32_t); i++) {
            buffer->buffer.push_back(((char*)&source.imm)[i]);
        }
        return;
    }

    assert(!"Cannot encode command");
}

} // namespace bonk::x86_backend