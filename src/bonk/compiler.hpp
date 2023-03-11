#pragma once

namespace bonk {

struct Compiler;
struct CompilerConfig;
struct Parser;

enum CompilerState {
    BONK_COMPILER_OK,
    BONK_COMPILER_STATE_ERROR,
    BONK_COMPILER_STATE_FATAL_ERROR
};

enum class CompilerMessageType { fatal_error, error, warning, note };

} // namespace bonk

#include <cstdio>
#include <sstream>
#include <ostream>
#include "backend/backend.hpp"
#include "parsing/parser.hpp"
#include "parsing/lexic/lexical_analyzer.hpp"
#include "tree/ast.hpp"
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

struct CompilerConfig {
    const OutputStream& error_file = NullOutputStream::instance;
    const OutputStream& listing_file = NullOutputStream::instance;
    const OutputStream& output_file = NullOutputStream::instance;
};

struct Compiler {
    const CompilerConfig config;

    CompilerState state = BONK_COMPILER_OK;
    Compiler();
    Compiler(const CompilerConfig& config);
    ~Compiler() = default;

    MessageStreamProxy error();
    MessageStreamProxy warning() const;
    MessageStreamProxy fatal_error();
};

} // namespace bonk

#include "parsing/parser.hpp"