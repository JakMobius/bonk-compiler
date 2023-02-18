#pragma once

#include "../x86_command_buffer.hpp"

namespace bonk::x86_backend {

struct scope_repeat_command : asm_command {

    scope_repeat_command();

    asm_command* clone() override {
        return new scope_repeat_command();
    }
};

} // namespace bonk::x86_backend
