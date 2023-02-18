#pragma once

#include "../../../../utils/linear_allocator.hpp"
#include "../x86_abstract_register.hpp"
#include "../x86_command_encoder.hpp"
#include "x86_command.hpp"

namespace bonk::x86_backend {

struct ret_command : asm_command {
    ret_command(abstract_register return_value_handle = -1);

    void to_bytes(command_encoder* buffer) override;

    asm_command* clone() override {
        if (parameters.size()) {
            return new ret_command(parameters[0].reg);
        } else {
            return new ret_command(-1);
        };
    }
};

} // namespace bonk::x86_backend
