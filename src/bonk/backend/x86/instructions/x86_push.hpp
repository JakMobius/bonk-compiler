#pragma once

#include "../x86_abstract_register.hpp"
#include "../x86_command_encoder.hpp"
#include "x86_command.hpp"

namespace bonk::x86_backend {

struct PushCommand : AsmCommand {
    explicit PushCommand(AbstractRegister reg);

    void to_bytes(CommandEncoder* buffer) override;

    AsmCommand* clone() override {
        return new PushCommand(parameters[0].reg);
    }
};

} // namespace bonk::x86_backend
