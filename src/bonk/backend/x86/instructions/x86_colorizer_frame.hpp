#pragma once

#include "x86_command.hpp"

namespace bonk::x86_backend {

struct frame_create_command : asm_command {
    frame_create_command();

    asm_command* clone() override {
        return new frame_create_command();
    }
};

struct frame_destroy_command : asm_command {
    frame_destroy_command();

    asm_command* clone() override {
        return new frame_destroy_command();
    }
};

} // namespace bonk::x86_backend
