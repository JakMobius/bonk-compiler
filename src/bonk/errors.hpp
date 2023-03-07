
#pragma once

namespace bonk {

struct Compiler;
struct Parser;
struct ParserPosition;
struct LexicalAnalyzer;

enum class CompilerMessageType { fatal_error, error, warning, note };

} // namespace bonk

#include <cstdarg>
#include <cstdio>
#include <optional>
#include <sstream>
#include "bonk/parsing/parser_position.hpp"
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

    MessageStreamProxy& at(const ParserPosition& message_position) {
        position = message_position;
        return *this;
    }

    friend std::ostream& operator<<(std::ostream& ostream, const MessageStreamProxy& proxy) {
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
};

} // namespace bonk
