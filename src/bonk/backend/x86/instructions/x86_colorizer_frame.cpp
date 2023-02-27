
#include "x86_colorizer_frame.hpp"

namespace bonk::x86_backend {

FrameCreateCommand::FrameCreateCommand() {
    type = COMMAND_COLORIZER_FRAME_CREATE;
}

FrameDestroyCommand::FrameDestroyCommand() {
    type = COMMAND_COLORIZER_FRAME_DESTROY;
}

} // namespace bonk::x86_backend