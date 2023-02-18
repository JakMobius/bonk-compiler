#pragma once

#include "x86_reg_stack.hpp"

namespace bonk::x86_backend {

struct backend_context_state;

}

namespace bonk::x86_backend {

struct backend_context_state {
    reg_stack register_stack;
    command_list* current_command_list;

    backend_context_state(command_list* current_command_list);
};

} // namespace bonk::x86_backend
