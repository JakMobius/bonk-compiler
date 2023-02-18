
#include "x86_imul.hpp"

namespace bonk::x86_backend {

imul_command::imul_command(abstract_register target, abstract_register source) {
    type = COMMAND_IMUL;

    parameters.resize(2);
    parameters[0] = command_parameter::create_register_64(target);
    parameters[1] = command_parameter::create_register_64(source);

    set_read_register(target);
    set_read_register(source);
    set_write_register(target);
}

void imul_command::to_bytes(command_encoder* buffer) {

    command_parameter target = parameters[0];
    command_parameter source = parameters[1];

    if (target.type == PARAMETER_TYPE_REG_64 && source.type == PARAMETER_TYPE_REG_64) {
        buffer->write_prefix_longopcode_regrm_sib(0x0F, 0xAF, target, source);

        return;
    }

    assert(!"Cannot encode command");
}

} // namespace bonk::x86_backend
