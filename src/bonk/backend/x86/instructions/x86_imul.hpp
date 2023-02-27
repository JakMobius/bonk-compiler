#pragma once

#include "../../../../utils/linear_allocator.hpp"
#include "../x86_abstract_register.hpp"
#include "x86_command.hpp"

namespace bonk::x86_backend {

struct IMulCommand : AsmCommand {
    IMulCommand(AbstractRegister target, AbstractRegister source);

    void to_bytes(CommandEncoder* buffer) override;

    AsmCommand* clone() override {
        return new IMulCommand(parameters[0].reg, parameters[1].reg);
    }
};

} // namespace bonk::x86_backend
