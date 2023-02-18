#pragma once

#include "../x86_abstract_register.hpp"
#include "../x86_command_encoder.hpp"
#include "x86_command.hpp"

namespace bonk::x86_backend {

struct push_command : asm_command {
    push_command(abstract_register reg);

    void to_bytes(command_encoder* buffer) override;

    asm_command* clone() override {
        return new push_command(parameters[0].reg);
    }
};

} // namespace bonk::x86_backend
