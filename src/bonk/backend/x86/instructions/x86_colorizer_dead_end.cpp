
#include "x86_colorizer_dead_end.hpp"

namespace bonk::x86_backend {

scope_dead_end_command::scope_dead_end_command() {
    type = COMMAND_COLORIZER_SCOPE_DEAD_END;
}

} // namespace bonk::x86_backend