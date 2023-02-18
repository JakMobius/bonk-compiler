

#pragma once

#include <stdbool.h>
#include <stdlib.h>
#include <cassert>

namespace bonk {

struct parser;

}

#include <vector>
#include "../compiler.hpp"
#include "./grammatics/grammatic_global.hpp"
#include "./lexic/lexical_analyzer.hpp"

namespace bonk {

struct parser {
    unsigned long position = 0;
    std::vector<lexeme>* input = nullptr;
    compiler* linked_compiler = nullptr;

    void warning(const char* format, ...);

    void error(const char* format, ...);

    void fatal_error(const char* format, ...);

    lexeme* next_lexeme();

    void eat_lexeme();

    void spit_lexeme();

    parser(compiler* compiler);

    tree_node_list<tree_node*>* parse_file(std::vector<lexeme>* lexemes);

    bool append_file(std::vector<lexeme>* lexemes, tree_node_list<tree_node*>* target);
};

} // namespace bonk
