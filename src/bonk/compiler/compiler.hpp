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
#include "bonk/backend/backend.hpp"
#include "bonk/frontend/parsing/lexic/lexer.hpp"
#include "bonk/frontend/parsing/parser.hpp"
#include "compiler_message_stream_proxy.hpp"
#include "bonk/frontend/ast/ast.hpp"
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

    CompilerMessageStreamProxy error();
    CompilerMessageStreamProxy warning() const;
    CompilerMessageStreamProxy fatal_error();

    void report_project_file(std::string_view path);
};

} // namespace bonk

#include "bonk/frontend/parsing/parser.hpp"