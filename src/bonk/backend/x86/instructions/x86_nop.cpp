
#include "x86_nop.hpp"

namespace bonk::x86_backend {

NopCommand::NopCommand() {
    type = COMMAND_NOP;
}

void NopCommand::to_bytes(CommandEncoder* buffer) {
    buffer->buffer.push_back(0x90);
}

} // namespace bonk::x86_backend