#pragma once

#include "../../../../utils/linear_allocator.hpp"
#include "../x86_abstract_register.hpp"
#include "../x86_command_encoder.hpp"
#include "x86_command.hpp"

namespace bonk::x86_backend {

struct nop_command : asm_command {

    nop_command();

    void to_bytes(command_encoder* buffer) override;

    asm_command* clone() override {
        return new nop_command();
    }
};

} // namespace bonk::x86_backend
