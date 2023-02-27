
#include "x86_ret.hpp"
#include "../x86_command_encoder.hpp"

namespace bonk::x86_backend {
RetCommand::RetCommand(AbstractRegister return_value_handle) {
    type = COMMAND_RET;

    if (return_value_handle != -1) {
        parameters.push_back(CommandParameter::create_register_64(return_value_handle));
        set_read_register(return_value_handle);
    }
}

void RetCommand::to_bytes(CommandEncoder* buffer) {
    // Note: return value handle is not handled here
    buffer->buffer.push_back(0xC3);
}
} // namespace bonk::x86_backend