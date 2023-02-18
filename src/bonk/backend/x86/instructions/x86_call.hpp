#pragma once

#include "../x86_backend_context.hpp"
#include "x86_command.hpp"

namespace bonk::x86_backend {

struct call_command : asm_command {
    call_command(command_parameter_symbol symbol, abstract_register* parameters, int amount);

    void to_bytes(command_encoder* buffer) override;

    asm_command* clone() override {
        return new call_command(parameters[0].symbol, &read_registers[0], read_registers.size());
    }
};

} // namespace bonk::x86_backend
