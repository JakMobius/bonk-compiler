
#include "x86_colorizer_align_stack.hpp"

namespace bonk::x86_backend {

AlignStackCommand* AlignStackCommand::create_before(int stack_entries) {
    return new AlignStackCommand(stack_entries, COMMAND_COLORIZER_ALIGN_STACK_BEFORE);
}

AlignStackCommand* AlignStackCommand::create_after(int stack_entries) {
    return new AlignStackCommand(stack_entries, COMMAND_COLORIZER_ALIGN_STACK_AFTER);
}

AlignStackCommand::AlignStackCommand(int stack_entries, AsmCommandType align_type) {
    type = align_type;

    parameters.resize(1);
    parameters[0] = CommandParameter::create_imm32(stack_entries);
}

int AlignStackCommand::get_stack_entries() {
    return parameters[0].imm;
}

} // namespace bonk::x86_backend