
#include "x86_idiv.hpp"

namespace bonk::x86_backend {

idiv_command::idiv_command(abstract_register divider, abstract_register rax_handle,
                           abstract_register rdx_handle) {
    type = COMMAND_IDIV;

    parameters.resize(1);
    parameters[0] = command_parameter::create_register_64(divider);

    set_read_register(divider);
    set_read_register(rax_handle);
    set_read_register(rdx_handle);
    set_write_register(rax_handle);
    set_write_register(rdx_handle);
}

void idiv_command::to_bytes(command_encoder* buffer) {

    command_parameter divider = parameters[0];

    if (divider.type == PARAMETER_TYPE_REG_64) {
        buffer->write_extended_opcode(0xF7, 7, divider);

        return;
    }

    assert(!"Cannot encode command");
}

} // namespace bonk::x86_backend
