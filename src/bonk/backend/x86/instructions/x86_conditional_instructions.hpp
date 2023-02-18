
#pragma once

#include "../x86_abstract_register.hpp"
#include "x86_command.hpp"
#include "x86_jmp_label.hpp"

namespace bonk::x86_backend {

struct jump_command : asm_command {
    void set_label(jmp_label* label);

    jmp_label* get_label();

    void invert_condition();

    jump_command(jmp_label* label, asm_command_type jump_type);

    void to_bytes(command_encoder* buffer) override;

    asm_command* clone() override {
        return new jump_command(get_label(), type);
    }
};

struct cset_command : asm_command {
    cset_command(abstract_register reg, asm_command_type cset_type);

    void to_bytes(command_encoder* buffer) override;

    asm_command* clone() override {
        return new cset_command(parameters[0].reg, type);
    }
};

} // namespace bonk::x86_backend
