#pragma once

#include "../x86_command_buffer.hpp"

namespace bonk::x86_backend {

struct ScopeDeadEndCommand : AsmCommand {

    ScopeDeadEndCommand();

    AsmCommand* clone() override {
        return new ScopeDeadEndCommand();
    }
};

} // namespace bonk::x86_backend
