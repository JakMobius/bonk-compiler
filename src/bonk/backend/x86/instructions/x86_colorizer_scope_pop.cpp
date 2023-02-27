
#include "x86_colorizer_scope_pop.hpp"

namespace bonk::x86_backend {

ScopePopCommand::ScopePopCommand(int scopes_to_pop) {
    type = COMMAND_COLORIZER_SCOPE_POP;
    parameters.resize(1);
    parameters[0] = CommandParameter::create_imm32(scopes_to_pop);
}

} // namespace bonk::x86_backend