#pragma once

#include "x86_command.hpp"

namespace bonk::x86_backend {

struct FrameCreateCommand : AsmCommand {
    FrameCreateCommand();

    AsmCommand* clone() override {
        return new FrameCreateCommand();
    }
};

struct FrameDestroyCommand : AsmCommand {
    FrameDestroyCommand();

    AsmCommand* clone() override {
        return new FrameDestroyCommand();
    }
};

} // namespace bonk::x86_backend
