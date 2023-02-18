
#include "x86_nop.hpp"

namespace bonk::x86_backend {

nop_command::nop_command() {
    type = COMMAND_NOP;
}

void nop_command::to_bytes(command_encoder* buffer) {
    buffer->buffer.push_back(0x90);
}

} // namespace bonk::x86_backend