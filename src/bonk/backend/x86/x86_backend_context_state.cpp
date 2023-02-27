
#include "x86_backend_context_state.hpp"

namespace bonk::x86_backend {

BackendContextState::BackendContextState(CommandList* command_list)
    : register_stack(command_list) {
    current_command_list = command_list;
}

} // namespace bonk::x86_backend