#pragma once

#include <sstream>
#include <optional>
#include "message_stream_proxy.hpp"
#include "compiler_message_type.hpp"
#include "parsing/parser_position.hpp"
#include "utils/streams.hpp"

namespace bonk {

struct MessageStreamProxy {
    CompilerMessageType message_type;
    std::stringstream message;
    const OutputStream& stream;
    std::optional<ParserPosition> position;

    MessageStreamProxy(CompilerMessageType message_type, const OutputStream& stream);

    ~MessageStreamProxy();

    template<typename T>
    MessageStreamProxy& operator<<(const T& str) {
        message << str;
        return *this;
    }

    MessageStreamProxy& at(const ParserPosition& message_position);

    friend std::ostream& operator<<(std::ostream& ostream, const MessageStreamProxy& proxy);
};

}
