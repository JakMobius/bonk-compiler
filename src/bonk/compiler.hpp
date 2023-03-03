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

} // namespace bonk

#include <cstdio>
#include <ostream>
#include "backend/backend.hpp"
#include "errors.hpp"
#include "parsing/parser.hpp"
#include "parsing/lexic/lexical_analyzer.hpp"
#include "tree/ast.hpp"
#include "utils/streams.hpp"

namespace bonk {

struct CompilerConfig {
    const OutputStream& error_file = NullOutputStream::instance;
    const OutputStream& listing_file = NullOutputStream::instance;
};

struct Compiler {
    const CompilerConfig& config;

    Parser parser;
    LexicalAnalyzer lexical_analyzer;

    CompilerState state = BONK_COMPILER_OK;

    Compiler(const CompilerConfig& config);

    ~Compiler() = default;

    MessageStreamProxy error();
    MessageStreamProxy warning() const;
    MessageStreamProxy fatal_error();
};

} // namespace bonk

#include "parsing/parser.hpp"