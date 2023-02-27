
#include "x86_xchg.hpp"
#include "../x86_command_encoder.hpp"

namespace bonk::x86_backend {

XchgCommand::XchgCommand(CommandParameter target, CommandParameter source) {
    type = COMMAND_XCHG;

    parameters.resize(2);
    parameters[0] = target;
    parameters[1] = source;

    assert(source.type == PARAMETER_TYPE_REG_64);

    set_read_register(source.reg);
    set_write_register(source.reg);

    if (target.type == PARAMETER_TYPE_REG_64) {
        set_read_register(target.reg);
        set_write_register(target.reg);
    }

    if (source.type == PARAMETER_TYPE_MEMORY) {
        for (int i = source.memory.register_amount() - 1; i >= 0; i--) {
            set_read_register(source.memory.get_register(i));
        }
    }
}

void XchgCommand::to_bytes(CommandEncoder* buffer) {

    CommandParameter target = parameters[0];
    CommandParameter source = parameters[1];

    if (target.type == PARAMETER_TYPE_REG_64 && source.type == PARAMETER_TYPE_REG_64) {
        buffer->write_prefix_opcode_modrm_sib(0x87, target, source);

        return;
    }

    if (target.type == PARAMETER_TYPE_REG_64 && source.type == PARAMETER_TYPE_MEMORY) {
        buffer->write_prefix_opcode_modrm_sib(0x87, target, source);

        return;
    }

    if (target.type == PARAMETER_TYPE_MEMORY && source.type == PARAMETER_TYPE_REG_64) {
        buffer->write_prefix_opcode_modrm_sib(0x87, source, target);

        return;
    }

    assert(!"Cannot encode command");
}

} // namespace bonk::x86_backend