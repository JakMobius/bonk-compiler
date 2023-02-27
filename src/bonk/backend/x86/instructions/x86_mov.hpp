#pragma once

namespace bonk::x86_backend {

struct MovCommand;

}

#include "../../../../utils/linear_allocator.hpp"
#include "../x86_abstract_register.hpp"
#include "../x86_backend_context.hpp"
#include "x86_command.hpp"

namespace bonk::x86_backend {

struct MovCommand : AsmCommand {
    MovCommand(CommandParameter target, CommandParameter source);

    void to_bytes(CommandEncoder* buffer) override;

    AsmCommand* clone() override {
        return new MovCommand(parameters[0], parameters[1]);
    }
};

struct MovZXCommand : MovCommand {
    MovZXCommand(CommandParameter target, CommandParameter source)
        : MovCommand(target, source) {
        type = COMMAND_MOVZX;
    }

    void to_bytes(CommandEncoder* buffer) override;

    AsmCommand* clone() override {
        return new MovZXCommand(parameters[0], parameters[1]);
    }
};

} // namespace bonk::x86_backend
