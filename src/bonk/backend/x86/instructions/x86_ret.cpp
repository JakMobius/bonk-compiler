
#include "x86_ret.hpp"
#include "../x86_command_encoder.hpp"

namespace bonk::x86_backend {
ret_command::ret_command(abstract_register return_value_handle) {
    type = COMMAND_RET;

    if (return_value_handle != -1) {
        parameters.push_back(command_parameter::create_register_64(return_value_handle));
        set_read_register(return_value_handle);
    }
}

void ret_command::to_bytes(command_encoder* buffer) {
    // Note: return value handle is not handled here
    buffer->buffer.push_back(0xC3);
}
} // namespace bonk::x86_backend