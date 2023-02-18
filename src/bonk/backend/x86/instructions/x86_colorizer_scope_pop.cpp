
#include "x86_colorizer_scope_pop.hpp"

namespace bonk::x86_backend {

scope_pop_command::scope_pop_command(int scopes_to_pop) {
    type = COMMAND_COLORIZER_SCOPE_POP;
    parameters.resize(1);
    parameters[0] = command_parameter::create_imm32(scopes_to_pop);
}

} // namespace bonk::x86_backend