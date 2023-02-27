#pragma once

#include "../../../../utils/linear_allocator.hpp"
#include "../x86_abstract_register.hpp"
#include "../x86_command_encoder.hpp"
#include "x86_command.hpp"

namespace bonk::x86_backend {

struct XorCommand : AsmCommand {
    XorCommand(CommandParameter target, CommandParameter source);

    void to_bytes(CommandEncoder* buffer) override;

    AsmCommand* clone() override {
        return new XorCommand(parameters[0], parameters[1]);
    }
};

} // namespace bonk::x86_backend
