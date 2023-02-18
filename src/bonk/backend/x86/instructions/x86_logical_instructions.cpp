
#include "x86_logical_instructions.hpp"

namespace bonk::x86_backend {

and_command::and_command(command_parameter target, command_parameter source) {
    type = COMMAND_AND;

    assert(target.type == PARAMETER_TYPE_REG_8 && source.type == PARAMETER_TYPE_REG_8);

    parameters.resize(2);
    parameters[0] = target;
    parameters[1] = source;

    set_read_register(source.reg);
    set_read_register(target.reg);
    set_write_register(target.reg);
}

void and_command::to_bytes(command_encoder* buffer) {

    command_parameter target = parameters[0];
    command_parameter source = parameters[1];

    if (target.type == PARAMETER_TYPE_REG_8 && source.type == PARAMETER_TYPE_REG_8) {
        buffer->write_prefix_opcode_modrm_sib(0x22, target, source);

        return;
    }

    assert(!"Cannot encode command");
}

or_command::or_command(command_parameter target, command_parameter source) {
    type = COMMAND_OR;

    assert(target.type == PARAMETER_TYPE_REG_8 && source.type == PARAMETER_TYPE_REG_8);

    parameters.resize(2);
    parameters[0] = target;
    parameters[1] = source;

    set_read_register(source.reg);
    set_read_register(target.reg);
    set_write_register(target.reg);
}

void or_command::to_bytes(command_encoder* buffer) {

    command_parameter target = parameters[0];
    command_parameter source = parameters[1];

    if (target.type == PARAMETER_TYPE_REG_8 && source.type == PARAMETER_TYPE_REG_8) {
        buffer->write_prefix_opcode_modrm_sib(0x0A, target, source);

        return;
    }

    assert(!"Cannot encode command");
}

} // namespace bonk::x86_backend