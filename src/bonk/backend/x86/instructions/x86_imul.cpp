
#include "x86_imul.hpp"

namespace bonk::x86_backend {

IMulCommand::IMulCommand(AbstractRegister target, AbstractRegister source) {
    type = COMMAND_IMUL;

    parameters.resize(2);
    parameters[0] = CommandParameter::create_register_64(target);
    parameters[1] = CommandParameter::create_register_64(source);

    set_read_register(target);
    set_read_register(source);
    set_write_register(target);
}

void IMulCommand::to_bytes(CommandEncoder* buffer) {

    CommandParameter target = parameters[0];
    CommandParameter source = parameters[1];

    if (target.type == PARAMETER_TYPE_REG_64 && source.type == PARAMETER_TYPE_REG_64) {
        buffer->write_prefix_longopcode_regrm_sib(0x0F, 0xAF, target, source);

        return;
    }

    assert(!"Cannot encode command");
}

} // namespace bonk::x86_backend
