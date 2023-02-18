#pragma once

#include "x86_command.hpp"

namespace bonk::x86_backend {

struct scope_command : asm_command {

    command_list* commands;

    scope_command(command_list* nested_list);

    asm_command* clone() override {
        return new scope_command(commands);
    }
};

} // namespace bonk::x86_backend
