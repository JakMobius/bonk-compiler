#pragma once

#include "../../../../utils/linear_allocator.hpp"
#include "../x86_abstract_register.hpp"
#include "x86_command.hpp"

namespace bonk::x86_backend {

struct imul_command : asm_command {
    imul_command(abstract_register target, abstract_register source);

    void to_bytes(command_encoder* buffer) override;

    asm_command* clone() override {
        return new imul_command(parameters[0].reg, parameters[1].reg);
    }
};

} // namespace bonk::x86_backend
