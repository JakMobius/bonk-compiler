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
#include "backend/backend.hpp"
#include "errors.hpp"
#include "parsing/lexic/lexical_analyzer.hpp"
#include "tree/ast.hpp"

namespace bonk {

struct CompilerConfig {
    FILE* error_file;
    FILE* listing_file;
    Backend* compile_backend;
};

struct Compiler {
    CompilerConfig* config = nullptr;

    Parser parser;
    LexicalAnalyzer lexical_analyzer;

    CompilerState state = BONK_COMPILER_OK;

    void out_of_memory();

    Compiler(CompilerConfig* config);

    ~Compiler();

    void fatal_error_positioned(ParserPosition* pPosition, const char* string, ...);

    void error_positioned(ParserPosition* pPosition, const char* string, ...);

    void error(const char* format, ...);

    void warning_positioned(ParserPosition* pos, const char* format, ...) const;

    void warning(const char* format, ...) const;

    void fatal_error(const char* format, ...);

    bool compile_ast(TreeNodeList* ast, FILE* target);
};

} // namespace bonk

#include "parsing/parser.hpp"