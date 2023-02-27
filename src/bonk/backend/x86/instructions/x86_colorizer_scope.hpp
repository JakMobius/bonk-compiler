#pragma once

#include "x86_command.hpp"

namespace bonk::x86_backend {

struct ScopeCommand : AsmCommand {

    CommandList* commands;

    explicit ScopeCommand(CommandList* nested_list);

    AsmCommand* clone() override {
        return new ScopeCommand(commands);
    }
};

} // namespace bonk::x86_backend
