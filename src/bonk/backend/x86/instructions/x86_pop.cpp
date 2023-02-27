
#include "x86_pop.hpp"
#include "../x86_command_encoder.hpp"

namespace bonk::x86_backend {

PopCommand::PopCommand(AbstractRegister reg) {
    type = COMMAND_POP;

    parameters.resize(1);
    parameters[0] = CommandParameter::create_register_64(reg);
    set_write_register(reg);
}

void PopCommand::to_bytes(CommandEncoder* buffer) {
    MachineRegister reg = buffer->to_machine_register(parameters[0].reg);
    RegisterExtensions rex = {.b = reg > 7};
    if (rex.exist())
        buffer->buffer.push_back(rex.get_byte());
    buffer->buffer.push_back(0x58 + (reg & 7));
}

} // namespace bonk::x86_backend