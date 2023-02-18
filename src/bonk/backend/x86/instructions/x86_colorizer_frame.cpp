
#include "x86_colorizer_frame.hpp"

namespace bonk::x86_backend {

frame_create_command::frame_create_command() {
    type = COMMAND_COLORIZER_FRAME_CREATE;
}

frame_destroy_command::frame_destroy_command() {
    type = COMMAND_COLORIZER_FRAME_DESTROY;
}

} // namespace bonk::x86_backend