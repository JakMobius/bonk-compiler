#pragma once

#include "../x86_command_buffer.hpp"

namespace bonk::x86_backend {

struct scope_dead_end_command : asm_command {

    scope_dead_end_command();

    asm_command* clone() override {
        return new scope_dead_end_command();
    }
};

} // namespace bonk::x86_backend
