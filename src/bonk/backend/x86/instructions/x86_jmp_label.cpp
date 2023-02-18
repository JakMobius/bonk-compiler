
#include "x86_jmp_label.hpp"

namespace bonk::x86_backend {

jmp_label::jmp_label(long long label) {
    type = COMMAND_JMP_LABEL;

    parameters.resize(1);
    parameters[0] = command_parameter::create_imm32(label);
}

unsigned long jmp_label::get_index() {
    return parameters[0].imm;
}

void jmp_label::to_bytes(command_encoder* buffer) {
}

} // namespace bonk::x86_backend