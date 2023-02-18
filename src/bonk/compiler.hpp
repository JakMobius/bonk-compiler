#pragma once

namespace bonk {

struct compiler;
struct compiler_config;

enum compiler_state {
    BONK_COMPILER_OK,
    BONK_COMPILER_STATE_ERROR,
    BONK_COMPILER_STATE_FATAL_ERROR
};

} // namespace bonk

#include <cstdio>
#include "backend/backend.hpp"
#include "errors.hpp"
#include "parsing/lexic/lexical_analyzer.hpp"
#include "parsing/parser.hpp"
#include "tree/nodes/nodes.hpp"

namespace bonk {

struct compiler_config {
    FILE* error_file;
    FILE* listing_file;
    backend* compile_backend;
};

struct compiler {
    compiler_config* config = nullptr;

    parser* parser = nullptr;
    lexical_analyzer* lexical_analyzer = nullptr;

    compiler_state state = BONK_COMPILER_OK;

    void out_of_memory();

    compiler(compiler_config* config);

    ~compiler();

    void fatal_error_positioned(parser_position* pPosition, const char* string, ...);

    void error_positioned(parser_position* pPosition, const char* string, ...);

    void error(const char* format, ...);

    void warning_positioned(parser_position* pos, const char* format, ...);

    void warning(const char* format, ...);

    void fatal_error(const char* format, ...);

    tree_node_list<tree_node*>* get_ast_of_file_at_path(const char* file_path);

    bool compile_ast(tree_node_list<tree_node*>* ast, FILE* target);
};

} // namespace bonk
