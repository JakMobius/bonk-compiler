

#pragma once

#include <cassert>
#include <cstdlib>

namespace bonk {

struct Parser;

}

#include <vector>
#include "../compiler.hpp"
#include "./lexic/lexical_analyzer.hpp"

namespace bonk {

struct Parser {
    unsigned long position = 0;
    std::vector<Lexeme>* input = nullptr;
    Compiler* linked_compiler = nullptr;

    void warning(const char* format, ...) const;

    void error(const char* format, ...) const;

    void fatal_error(const char* format, ...) const;

    Lexeme* next_lexeme();

    void eat_lexeme();

    void spit_lexeme();

    Parser(Compiler* compiler);

    TreeNodeList* parse_file(std::vector<Lexeme>* lexemes);

    bool append_file(std::vector<Lexeme>* lexemes, TreeNodeList* target);

  private:
    TreeNode* parse_code_line();
    TreeNode* parse_assignment();
    TreeNode* parse_unary_operator();
    TreeNode* parse_sub_block();
    TreeNodeList* parse_nested_block();
    TreeNodeCycle* parse_cycle();
    TreeNodeCheck* parse_check();
    TreeNodeVariableDefinition* parse_var_definition();
    TreeNode* parse_reference();
    TreeNodeList* parse_arguments();
    bool parse_program(TreeNodeList* target);
    TreeNode* parse_math_term();
    TreeNode* parse_math_factor();
    TreeNode* parse_math_expression();
    TreeNode* parse_global_definition();
    TreeNode* parse_expression();
    TreeNode* parse_logic_term();
    TreeNode* parse_logic_expression();
    TreeNode* parse_comparison();
    TreeNode* parse_bams();
    bool parse_help(TreeNodeList* target);
    bool parse_global(TreeNodeList* target);
    TreeNode* parse_expression_leveled(bool top_level);
    TreeNodeBlockDefinition* parse_block_definition();
    TreeNodeList* parse_block();


    bool is_comparison_operator(OperatorType oper);
};

} // namespace bonk
