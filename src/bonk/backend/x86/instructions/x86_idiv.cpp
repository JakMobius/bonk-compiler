
#include "x86_idiv.hpp"

namespace bonk::x86_backend {

IDivCommand::IDivCommand(AbstractRegister divider, AbstractRegister rax_handle,
                         AbstractRegister rdx_handle) {
    type = COMMAND_IDIV;

    parameters.resize(1);
    parameters[0] = CommandParameter::create_register_64(divider);

    set_read_register(divider);
    set_read_register(rax_handle);
    set_read_register(rdx_handle);
    set_write_register(rax_handle);
    set_write_register(rdx_handle);
}

void IDivCommand::to_bytes(CommandEncoder* buffer) {

    CommandParameter divider = parameters[0];

    if (divider.type == PARAMETER_TYPE_REG_64) {
        buffer->write_extended_opcode(0xF7, 7, divider);

        return;
    }

    assert(!"Cannot encode command");
}

} // namespace bonk::x86_backend
