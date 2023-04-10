#pragma once

#include <optional>
#include <sstream>
#include "bonk/frontend/parsing/parser_position.hpp"
#include "compiler_message_stream_proxy.hpp"
#include "utils/streams.hpp"

namespace bonk {

enum class CompilerMessageType { fatal_error, error, warning, note };

struct CompilerMessageStreamProxy {
    CompilerMessageType message_type;
    std::stringstream message;
    const OutputStream& stream;
    std::optional<ParserPosition> position;

    CompilerMessageStreamProxy(CompilerMessageType message_type, const OutputStream& stream);

    ~CompilerMessageStreamProxy();

    template<typename T> CompilerMessageStreamProxy& operator<<(const T& str) {
        message << str;
        return *this;
    }

    CompilerMessageStreamProxy& at(const ParserPosition& message_position);

    friend std::ostream& operator<<(std::ostream& ostream, const CompilerMessageStreamProxy& proxy);
};

}
