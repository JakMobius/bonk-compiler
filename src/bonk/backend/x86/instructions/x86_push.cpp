
#include "x86_push.hpp"

namespace bonk::x86_backend {

push_command::push_command(abstract_register reg) {
    type = COMMAND_PUSH;

    parameters.resize(1);
    parameters[0] = command_parameter::create_register_64(reg);
    set_read_register(reg);
}

void push_command::to_bytes(command_encoder* buffer) {
    e_machine_register reg = buffer->to_machine_register(parameters[0].reg);
    register_extensions rex = {.b = reg > 7};
    if (rex.exist())
        buffer->buffer.push_back(rex.get_byte());
    buffer->buffer.push_back(0x50 + (reg & 7));
}

} // namespace bonk::x86_backend