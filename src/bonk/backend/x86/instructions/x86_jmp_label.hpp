#pragma once

#include "../../../../utils/linear_allocator.hpp"
#include "../x86_abstract_register.hpp"
#include "../x86_command_encoder.hpp"
#include "x86_command.hpp"

namespace bonk::x86_backend {

struct JmpLabel : AsmCommand {

    explicit JmpLabel(long long label);

    void to_bytes(CommandEncoder* buffer) override;

    unsigned long get_index();

    AsmCommand* clone() override {
        return new JmpLabel(parameters[0].imm);
    }
};

} // namespace bonk::x86_backend
