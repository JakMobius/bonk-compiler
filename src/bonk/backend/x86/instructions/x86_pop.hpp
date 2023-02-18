#pragma once

#include "../x86_abstract_register.hpp"
#include "../x86_command_encoder.hpp"
#include "x86_command.hpp"

namespace bonk::x86_backend {

struct pop_command : asm_command {
    pop_command(abstract_register reg);

    void to_bytes(command_encoder* buffer) override;

    asm_command* clone() override {
        return new pop_command(parameters[0].reg);
    }
};

} // namespace bonk::x86_backend
