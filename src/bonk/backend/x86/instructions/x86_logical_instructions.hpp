#pragma once

#include "../../../../utils/linear_allocator.hpp"
#include "../x86_abstract_register.hpp"
#include "../x86_command_encoder.hpp"
#include "x86_command.hpp"

namespace bonk::x86_backend {

struct and_command : asm_command {
    and_command(command_parameter target, command_parameter source);

    void to_bytes(command_encoder* buffer) override;

    asm_command* clone() override {
        return new and_command(parameters[0], parameters[1]);
    }
};

struct or_command : asm_command {
    or_command(command_parameter target, command_parameter source);

    void to_bytes(command_encoder* buffer) override;

    asm_command* clone() override {
        return new or_command(parameters[0], parameters[1]);
    }
};

} // namespace bonk::x86_backend
