#pragma once

#include "../x86_command_buffer.hpp"

namespace bonk::x86_backend {

struct align_stack_command : asm_command {

    align_stack_command(int stack_entries, asm_command_type type);

    static align_stack_command* create_before(int stack_entries);

    static align_stack_command* create_after(int stack_entries);

    int get_stack_entries();

    asm_command* clone() override {
        return new align_stack_command(parameters[0].imm, type);
    }
};

} // namespace bonk::x86_backend
