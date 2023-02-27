

#pragma once

#include <stdbool.h>
#include <stdlib.h>
#include <cassert>

namespace bonk {

struct Parser;

}

#include <vector>
#include "../compiler.hpp"
#include "./grammatics/grammatic_global.hpp"
#include "./lexic/lexical_analyzer.hpp"

namespace bonk {

struct Parser {
    unsigned long position = 0;
    std::vector<Lexeme>* input = nullptr;
    Compiler* linked_compiler = nullptr;

    void warning(const char* format, ...);

    void error(const char* format, ...);

    void fatal_error(const char* format, ...);

    Lexeme* next_lexeme();

    void eat_lexeme();

    void spit_lexeme();

    Parser(Compiler* compiler);

    TreeNodeList<TreeNode*>* parse_file(std::vector<Lexeme>* lexemes);

    bool append_file(std::vector<Lexeme>* lexemes, TreeNodeList<TreeNode*>* target);
};

} // namespace bonk
