#pragma once

namespace bonk {

struct Compiler;
struct CompilerConfig;
struct Parser;

} // namespace bonk

#include <cstdio>
#include <ostream>
#include <sstream>
#include <unordered_set>
#include "backend/backend.hpp"
#include "bonk/parsing/lexic/lexer.hpp"
#include "message_stream_proxy.hpp"
#include "parsing/parser.hpp"
#include "tree/ast.hpp"
#include "utils/streams.hpp"

namespace bonk {

struct CompilerConfig {
    const OutputStream& error_file = NullOutputStream::instance;
};

struct Compiler {
    const CompilerConfig config;
    Backend* backend = nullptr;

    std::unordered_set<std::string> updated_files;
    std::unordered_set<std::string> output_files;

    Compiler();
    Compiler(const CompilerConfig& config);
    ~Compiler() = default;

    MessageStreamProxy error();
    MessageStreamProxy warning() const;
    MessageStreamProxy fatal_error();

    void report_project_file(std::string_view path);
};

} // namespace bonk

#include "parsing/parser.hpp"