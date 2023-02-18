
#include "x86_pop.hpp"
#include "../x86_command_encoder.hpp"

namespace bonk::x86_backend {

pop_command::pop_command(abstract_register reg) {
    type = COMMAND_POP;

    parameters.resize(1);
    parameters[0] = command_parameter::create_register_64(reg);
    set_write_register(reg);
}

void pop_command::to_bytes(command_encoder* buffer) {
    e_machine_register reg = buffer->to_machine_register(parameters[0].reg);
    register_extensions rex = {.b = reg > 7};
    if (rex.exist())
        buffer->buffer.push_back(rex.get_byte());
    buffer->buffer.push_back(0x58 + (reg & 7));
}

} // namespace bonk::x86_backend