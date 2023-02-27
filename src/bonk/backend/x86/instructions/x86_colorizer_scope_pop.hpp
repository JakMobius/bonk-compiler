#pragma once

#include "../x86_command_buffer.hpp"

namespace bonk::x86_backend {

struct ScopePopCommand : AsmCommand {

    ScopePopCommand(int scopes_to_pop);

    AsmCommand* clone() override {
        return new ScopePopCommand(parameters[0].imm);
    }
};

} // namespace bonk::x86_backend
