#pragma once

#include "../../../../utils/linear_allocator.hpp"
#include "../x86_abstract_register.hpp"
#include "../x86_command_encoder.hpp"
#include "x86_command.hpp"

namespace bonk::x86_backend {

struct jmp_label : asm_command {

    unsigned long jmps_targeting;

    jmp_label(long long label);

    void to_bytes(command_encoder* buffer) override;

    unsigned long get_index();

    asm_command* clone() override {
        return new jmp_label(parameters[0].imm);
    }
};

} // namespace bonk::x86_backend
