#pragma once

#include "../x86_command_buffer.hpp"

namespace bonk::x86_backend {

struct AlignStackCommand : AsmCommand {

    AlignStackCommand(int stack_entries, AsmCommandType type);

    static AlignStackCommand* create_before(int stack_entries);

    static AlignStackCommand* create_after(int stack_entries);

    int get_stack_entries();

    AsmCommand* clone() override {
        return new AlignStackCommand(parameters[0].imm, type);
    }
};

} // namespace bonk::x86_backend
