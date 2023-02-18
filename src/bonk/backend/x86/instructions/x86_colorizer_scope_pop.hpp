#pragma once

#include "../x86_command_buffer.hpp"

namespace bonk::x86_backend {

struct scope_pop_command : asm_command {

    scope_pop_command(int scopes_to_pop);

    asm_command* clone() override {
        return new scope_pop_command(parameters[0].imm);
    }
};

} // namespace bonk::x86_backend
