#pragma once

#include "../../../../utils/linear_allocator.hpp"
#include "../x86_abstract_register.hpp"
#include "../x86_command_encoder.hpp"
#include "x86_command.hpp"

namespace bonk::x86_backend {

struct NopCommand : AsmCommand {

    NopCommand();

    void to_bytes(CommandEncoder* buffer) override;

    AsmCommand* clone() override {
        return new NopCommand();
    }
};

} // namespace bonk::x86_backend
