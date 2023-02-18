#pragma once

namespace bonk::x86_backend {

struct mov_command;

}

#include "../../../../utils/linear_allocator.hpp"
#include "../x86_abstract_register.hpp"
#include "../x86_backend_context.hpp"
#include "x86_command.hpp"

namespace bonk::x86_backend {

struct mov_command : asm_command {
    mov_command(command_parameter target, command_parameter source);

    void to_bytes(command_encoder* buffer) override;

    asm_command* clone() override {
        return new mov_command(parameters[0], parameters[1]);
    }
};

struct movzx_command : mov_command {
    movzx_command(command_parameter target, command_parameter source)
        : mov_command(target, source) {
        type = COMMAND_MOVZX;
    }

    void to_bytes(command_encoder* buffer) override;

    asm_command* clone() override {
        return new movzx_command(parameters[0], parameters[1]);
    }
};

} // namespace bonk::x86_backend
