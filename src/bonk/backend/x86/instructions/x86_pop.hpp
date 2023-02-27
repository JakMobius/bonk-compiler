#pragma once

#include "../x86_abstract_register.hpp"
#include "../x86_command_encoder.hpp"
#include "x86_command.hpp"

namespace bonk::x86_backend {

struct PopCommand : AsmCommand {
    PopCommand(AbstractRegister reg);

    void to_bytes(CommandEncoder* buffer) override;

    AsmCommand* clone() override {
        return new PopCommand(parameters[0].reg);
    }
};

} // namespace bonk::x86_backend
