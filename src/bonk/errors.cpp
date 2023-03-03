
#include "errors.hpp"
#include "compiler.hpp"
#include "parsing/lexic/lexical_analyzer.hpp"

namespace bonk {

MessageStreamProxy Compiler::warning() const {
    return {CompilerMessageType::warning, config.error_file};
}

MessageStreamProxy Compiler::error() {
    state = BONK_COMPILER_STATE_ERROR;
    return {CompilerMessageType::error, config.error_file};
}

MessageStreamProxy Compiler::fatal_error() {
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
} // namespace bonk