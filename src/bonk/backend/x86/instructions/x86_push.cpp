
#include "x86_push.hpp"

namespace bonk::x86_backend {

PushCommand::PushCommand(AbstractRegister reg) {
    type = COMMAND_PUSH;

    parameters.resize(1);
    parameters[0] = CommandParameter::create_register_64(reg);
    set_read_register(reg);
}

void PushCommand::to_bytes(CommandEncoder* buffer) {
    MachineRegister reg = buffer->to_machine_register(parameters[0].reg);
    RegisterExtensions rex = {.b = reg > 7};
    if (rex.exist())
        buffer->buffer.push_back(rex.get_byte());
    buffer->buffer.push_back(0x50 + (reg & 7));
}

} // namespace bonk::x86_backend