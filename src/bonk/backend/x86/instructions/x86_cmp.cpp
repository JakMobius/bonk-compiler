
#include "x86_cmp.hpp"

namespace bonk::x86_backend {

CmpCommand::CmpCommand(CommandParameter target, CommandParameter source) {
    type = COMMAND_CMP;

    assert(target.type == PARAMETER_TYPE_REG_64 || target.type == PARAMETER_TYPE_REG_8);

    parameters.resize(2);
    parameters[0] = target;
    parameters[1] = source;

    set_read_register(source.reg);
    set_read_register(target.reg);
}

void CmpCommand::to_bytes(CommandEncoder* buffer) {

    CommandParameter target = parameters[0];
    CommandParameter source = parameters[1];

    if (target.type == PARAMETER_TYPE_REG_64 && source.type == PARAMETER_TYPE_REG_64) {
        buffer->write_prefix_opcode_modrm_sib(0x3B, target, source);

        return;
    }

    if (target.type == PARAMETER_TYPE_REG_8 && source.type == PARAMETER_TYPE_REG_8) {
        buffer->write_prefix_opcode_modrm_sib(0x38, target, source);

        return;
    }

    assert(!"Cannot encode command");
}

} // namespace bonk::x86_backend
