#pragma once

#include "../../../../utils/linear_allocator.hpp"
#include "../x86_abstract_register.hpp"
#include "../x86_command_encoder.hpp"
#include "x86_command.hpp"

namespace bonk::x86_backend {

struct RetCommand : AsmCommand {
    RetCommand(AbstractRegister return_value_handle = -1);

    void to_bytes(CommandEncoder* buffer) override;

    AsmCommand* clone() override {
        if (parameters.size()) {
            return new RetCommand(parameters[0].reg);
        } else {
            return new RetCommand(-1);
        };
    }
};

} // namespace bonk::x86_backend
