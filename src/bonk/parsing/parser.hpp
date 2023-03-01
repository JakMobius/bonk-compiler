

#pragma once

#include <cassert>
#include <cstdlib>

namespace bonk {

struct Parser;

}

#include <vector>
#include <optional>
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
    std::unique_ptr<TreeNode> parse_code_line();
    std::unique_ptr<TreeNode> parse_assignment();
    std::unique_ptr<TreeNode> parse_unary_operator();
    std::unique_ptr<TreeNode> parse_sub_block();
    std::unique_ptr<TreeNodeList> parse_nested_block();
    std::unique_ptr<TreeNodeCycle> parse_cycle();
    std::unique_ptr<TreeNodeCheck> parse_check();
    std::unique_ptr<TreeNodeVariableDefinition> parse_var_definition();
    std::unique_ptr<TreeNode> parse_reference();
    std::unique_ptr<TreeNodeList> parse_arguments();
    std::unique_ptr<TreeNode> parse_math_term();
    std::unique_ptr<TreeNode> parse_math_factor();
    std::unique_ptr<TreeNode> parse_math_expression();
    std::unique_ptr<TreeNode> parse_global_definition();
    std::unique_ptr<TreeNode> parse_expression();
    std::unique_ptr<TreeNode> parse_logic_term();
    std::unique_ptr<TreeNode> parse_logic_expression();
    std::unique_ptr<TreeNode> parse_comparison();
    std::unique_ptr<TreeNode> parse_bams();
    std::unique_ptr<TreeNode> parse_expression_leveled(bool top_level);
    std::unique_ptr<TreeNodeBlockDefinition> parse_block_definition();
    std::unique_ptr<TreeNodeList> parse_block();

    bool parse_help(TreeNodeList* target);
    bool parse_global(TreeNodeList* target);
    bool parse_program(TreeNodeList* target);

    bool is_comparison_operator(OperatorType oper);
};

} // namespace bonk
