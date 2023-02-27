#pragma once

#include "../x86_command_buffer.hpp"

namespace bonk::x86_backend {

struct ScopeRepeatCommand : AsmCommand {

    ScopeRepeatCommand();

    AsmCommand* clone() override {
        return new ScopeRepeatCommand();
    }
};

} // namespace bonk::x86_backend
