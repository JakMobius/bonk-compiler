
#include "x86_colorizer_align_stack.hpp"

namespace bonk::x86_backend {

align_stack_command* align_stack_command::create_before(int stack_entries) {
    return new align_stack_command(stack_entries, COMMAND_COLORIZER_ALIGN_STACK_BEFORE);
}

align_stack_command* align_stack_command::create_after(int stack_entries) {
    return new align_stack_command(stack_entries, COMMAND_COLORIZER_ALIGN_STACK_AFTER);
}

align_stack_command::align_stack_command(int stack_entries, asm_command_type align_type) {
    type = align_type;

    parameters.resize(1);
    parameters[0] = command_parameter::create_imm32(stack_entries);
}

int align_stack_command::get_stack_entries() {
    return parameters[0].imm;
}

} // namespace bonk::x86_backend