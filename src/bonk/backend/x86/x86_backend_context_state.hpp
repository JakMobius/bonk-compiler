#pragma once

#include "x86_reg_stack.hpp"

namespace bonk::x86_backend {

struct BackendContextState;

}

namespace bonk::x86_backend {

struct BackendContextState {
    RegStack register_stack;
    CommandList* current_command_list;

    explicit BackendContextState(CommandList* current_command_list);
};

} // namespace bonk::x86_backend
