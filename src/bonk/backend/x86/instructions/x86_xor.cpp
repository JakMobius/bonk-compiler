
#include "x86_xor.hpp"

namespace bonk::x86_backend {

xor_command::xor_command(command_parameter target, command_parameter source) {
    type = COMMAND_XOR;

    assert(target.type == PARAMETER_TYPE_REG_64 && source.type == PARAMETER_TYPE_REG_64);

    parameters.resize(2);
    parameters[0] = target;
    parameters[1] = source;

    set_read_register(target.reg);
    set_write_register(target.reg);
    set_read_register(source.reg);
}

void xor_command::to_bytes(command_encoder* buffer) {

    command_parameter target = parameters[0];
    command_parameter source = parameters[1];

    if (target.type == PARAMETER_TYPE_REG_64 && source.type == PARAMETER_TYPE_REG_64) {
        buffer->write_prefix_opcode_modrm_sib(0x31, source, target);

        return;
    }

    assert(!"Cannot encode command");
}

} // namespace bonk::x86_backend