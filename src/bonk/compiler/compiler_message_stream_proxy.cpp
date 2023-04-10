
#include "compiler_message_stream_proxy.hpp"

namespace bonk {

CompilerMessageStreamProxy::CompilerMessageStreamProxy(CompilerMessageType message_type, const OutputStream& stream)
    : message_type(message_type), stream(stream) {
}

CompilerMessageStreamProxy::~CompilerMessageStreamProxy() {
    if (stream) {
        stream.get_stream() << *this;
    }
}

CompilerMessageStreamProxy& CompilerMessageStreamProxy::at(const ParserPosition& message_position) {
    position = message_position;
    return *this;
}

std::ostream& operator<<(std::ostream& ostream, const CompilerMessageStreamProxy& proxy) {
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

}