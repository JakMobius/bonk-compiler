
#include "x86_colorizer_repeat_scope.hpp"

namespace bonk::x86_backend {

scope_repeat_command::scope_repeat_command() {
    type = COMMAND_COLORIZER_REPEAT_SCOPE;
}

} // namespace bonk::x86_backend