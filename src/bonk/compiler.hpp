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
#include <ostream>
#include <sstream>
#include <unordered_set>
#include "backend/backend.hpp"
#include "bonk/parsing/lexic/lexer.hpp"
#include "parsing/parser.hpp"
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
};

struct Compiler {
    const CompilerConfig config;
    Backend* backend = nullptr;

    std::unordered_set<std::string> updated_files;
    std::unordered_set<std::string> output_files;

    CompilerState state = BONK_COMPILER_OK;
    Compiler();
    Compiler(const CompilerConfig& config);
    ~Compiler() = default;

    MessageStreamProxy error();
    MessageStreamProxy warning() const;
    MessageStreamProxy fatal_error();

    void report_file_updated(std::string_view path);
    void report_project_file(std::string_view path);
};

} // namespace bonk

#include "parsing/parser.hpp"