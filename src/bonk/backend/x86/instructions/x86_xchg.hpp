#pragma once

namespace bonk::x86_backend {

struct XchgCommand;

}

#include "../../../../utils/linear_allocator.hpp"
#include "../x86_abstract_register.hpp"
#include "../x86_backend_context.hpp"
#include "../x86_command_encoder.hpp"
#include "x86_command.hpp"

namespace bonk::x86_backend {

struct XchgCommand : AsmCommand {
    XchgCommand(CommandParameter target, CommandParameter source);

    void to_bytes(CommandEncoder* buffer) override;

    AsmCommand* clone() override {
        return new XchgCommand(parameters[0], parameters[1]);
    }
};

} // namespace bonk::x86_backend
