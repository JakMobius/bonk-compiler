
#pragma once

#include "../../../../utils/linear_allocator.hpp"
#include "../x86_abstract_register.hpp"
#include "x86_command.hpp"

namespace bonk::x86_backend {

struct sub_command : asm_command {
    sub_command(command_parameter target, command_parameter source);

    void to_bytes(command_encoder* buffer) override;

    asm_command* clone() override {
        return new sub_command(parameters[0], parameters[1]);
    }
};

} // namespace bonk::x86_backend
