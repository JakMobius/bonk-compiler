
#include "x86_xor.hpp"

namespace bonk::x86_backend {

XorCommand::XorCommand(CommandParameter target, CommandParameter source) {
    type = COMMAND_XOR;

    assert(target.type == PARAMETER_TYPE_REG_64 && source.type == PARAMETER_TYPE_REG_64);

    parameters.resize(2);
    parameters[0] = target;
    parameters[1] = source;

    set_read_register(target.reg);
    set_write_register(target.reg);
    set_read_register(source.reg);
}

void XorCommand::to_bytes(CommandEncoder* buffer) {

    CommandParameter target = parameters[0];
    CommandParameter source = parameters[1];

    if (target.type == PARAMETER_TYPE_REG_64 && source.type == PARAMETER_TYPE_REG_64) {
        buffer->write_prefix_opcode_modrm_sib(0x31, source, target);

        return;
    }

    assert(!"Cannot encode command");
}

} // namespace bonk::x86_backend