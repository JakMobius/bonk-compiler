
#pragma once

#include "../x86_abstract_register.hpp"
#include "x86_command.hpp"
#include "x86_jmp_label.hpp"

namespace bonk::x86_backend {

struct JumpCommand : AsmCommand {
    void set_label(JmpLabel* label);

    JmpLabel* get_label();

    void invert_condition();

    JumpCommand(JmpLabel* label, AsmCommandType jump_type);

    void to_bytes(CommandEncoder* buffer) override;

    AsmCommand* clone() override {
        return new JumpCommand(get_label(), type);
    }
};

struct CSetCommand : AsmCommand {
    CSetCommand(AbstractRegister reg, AsmCommandType cset_type);

    void to_bytes(CommandEncoder* buffer) override;

    AsmCommand* clone() override {
        return new CSetCommand(parameters[0].reg, type);
    }
};

} // namespace bonk::x86_backend
