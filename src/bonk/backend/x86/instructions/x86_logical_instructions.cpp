
#include "x86_logical_instructions.hpp"

namespace bonk::x86_backend {

AndCommand::AndCommand(CommandParameter target, CommandParameter source) {
    type = COMMAND_AND;

    assert(target.type == PARAMETER_TYPE_REG_8 && source.type == PARAMETER_TYPE_REG_8);

    parameters.resize(2);
    parameters[0] = target;
    parameters[1] = source;

    set_read_register(source.reg);
    set_read_register(target.reg);
    set_write_register(target.reg);
}

void AndCommand::to_bytes(CommandEncoder* buffer) {

    CommandParameter target = parameters[0];
    CommandParameter source = parameters[1];

    if (target.type == PARAMETER_TYPE_REG_8 && source.type == PARAMETER_TYPE_REG_8) {
        buffer->write_prefix_opcode_modrm_sib(0x22, target, source);

        return;
    }

    assert(!"Cannot encode command");
}

OrCommand::OrCommand(CommandParameter target, CommandParameter source) {
    type = COMMAND_OR;

    assert(target.type == PARAMETER_TYPE_REG_8 && source.type == PARAMETER_TYPE_REG_8);

    parameters.resize(2);
    parameters[0] = target;
    parameters[1] = source;

    set_read_register(source.reg);
    set_read_register(target.reg);
    set_write_register(target.reg);
}

void OrCommand::to_bytes(CommandEncoder* buffer) {

    CommandParameter target = parameters[0];
    CommandParameter source = parameters[1];

    if (target.type == PARAMETER_TYPE_REG_8 && source.type == PARAMETER_TYPE_REG_8) {
        buffer->write_prefix_opcode_modrm_sib(0x0A, target, source);

        return;
    }

    assert(!"Cannot encode command");
}

} // namespace bonk::x86_backend