#pragma once

#include "../x86_backend_context.hpp"
#include "x86_command.hpp"

namespace bonk::x86_backend {

struct CallCommand : AsmCommand {
    CallCommand(CommandParameterSymbol symbol, AbstractRegister* parameters, int amount);

    void to_bytes(CommandEncoder* buffer) override;

    AsmCommand* clone() override {
        return new CallCommand(parameters[0].symbol, &read_registers[0], read_registers.size());
    }
};

} // namespace bonk::x86_backend
