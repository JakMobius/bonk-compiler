
#include "x86_jmp_label.hpp"

namespace bonk::x86_backend {

JmpLabel::JmpLabel(long long label) {
    type = COMMAND_JMP_LABEL;

    parameters.resize(1);
    parameters[0] = CommandParameter::create_imm32(label);
}

unsigned long JmpLabel::get_index() {
    return parameters[0].imm;
}

void JmpLabel::to_bytes(CommandEncoder* buffer) {
}

} // namespace bonk::x86_backend