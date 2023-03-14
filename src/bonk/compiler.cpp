
#include "compiler.hpp"

namespace bonk {

Compiler::Compiler() : config() {
}

Compiler::Compiler(const CompilerConfig& config) : config(config) {
}

MessageStreamProxy Compiler::warning() const {
    return {CompilerMessageType::warning, config.error_file};
}

MessageStreamProxy Compiler::error() {
    state = BONK_COMPILER_STATE_ERROR;
    return {CompilerMessageType::error, config.error_file};
}

MessageStreamProxy Compiler::fatal_error() {
    state = BONK_COMPILER_STATE_FATAL_ERROR;
    return {CompilerMessageType::fatal_error, config.error_file};
}

MessageStreamProxy::MessageStreamProxy(CompilerMessageType message_type, const OutputStream& stream)
    : message_type(message_type), stream(stream) {
}

MessageStreamProxy::~MessageStreamProxy() {
    if (stream) {
        stream.get_stream() << *this;
    }
}

MessageStreamProxy& MessageStreamProxy::at(const ParserPosition& message_position) {
    position = message_position;
    return *this;
}

std::ostream& operator<<(std::ostream& ostream, const MessageStreamProxy& proxy) {
    if (proxy.position.has_value()) {
        ostream << proxy.position.value() << ": ";
    }

    switch (proxy.message_type) {
    case CompilerMessageType::error:
        ostream << "error: ";
        break;
    case CompilerMessageType::warning:
        ostream << "warning: ";
        break;
    case CompilerMessageType::note:
        ostream << "note: ";
        break;
    case CompilerMessageType::fatal_error:
        ostream << "fatal error: ";
        break;
    }

    ostream << proxy.message.str() << std::endl;
    return ostream;
}

bool CompilerConfig::should_stop_after(std::string_view stage) const {
    if (!stop_checkpoint.has_value())
        return false;
    auto& checkpoint = stop_checkpoint.value();

    // Check if checkpoint has prefix of stage
    if (stage.size() > checkpoint.size())
        return false;

    for (size_t i = 0; i < stage.size(); i++) {
        if (stage[i] != checkpoint[i])
            return false;
    }

    return true;
}

} // namespace bonk