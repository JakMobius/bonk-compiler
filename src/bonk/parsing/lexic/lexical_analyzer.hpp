
#pragma once

namespace bonk {

struct Lexeme;
struct LexicalAnalyzer;
struct ParserPosition;
struct Compiler;

enum LexemeType {
    BONK_LEXEME_NULL,
    BONK_LEXEME_KEYWORD,
    BONK_LEXEME_IDENTIFIER,
    BONK_LEXEME_NUMBER,
    BONK_LEXEME_SEMICOLON,
    BONK_LEXEME_OPERATOR,
    BONK_LEXEME_BRACE,
    BONK_LEXEME_COMMA,
    BONK_LEXEME_CALL,
    BONK_LEXEME_INLINE_BAMS
};

enum KeywordType {
    BONK_KEYWORD_VAR,
    BONK_KEYWORD_BLOCK,
    BONK_KEYWORD_PRINT,
    BONK_KEYWORD_CONTEXT,
    BONK_KEYWORD_BONK,
    BONK_KEYWORD_CHECK,
    BONK_KEYWORD_CYCLE,
    BONK_KEYWORD_BREK,
    BONK_KEYWORD_BAMS,
    BONK_KEYWORD_AND,
    BONK_KEYWORD_OR,
    BONK_KEYWORD_REBONK,
    BONK_KEYWORD_HELP,
    BONK_KEYWORD_PROMISE
};

enum BraceType {
    BONK_BRACE_L_CB,
    BONK_BRACE_R_CB,
    BONK_BRACE_L_RB,
    BONK_BRACE_R_RB,
    BONK_BRACE_L_SB,
    BONK_BRACE_R_SB
};

} // namespace bonk

#include <vector>
#include <cctype>
#include "bonk/tree/ast.hpp"
#include "../parser_position.hpp"
#include "identifier_lexeme.hpp"
#include "number_lexeme.hpp"

namespace bonk {

extern const char* BONK_OPERATOR_NAMES[];
extern const char* BONK_KEYWORD_NAMES[];

struct NumberLexeme {
    long long int integer_value;
    long double double_value;
};

struct Lexeme {
    ParserPosition* position;
    LexemeType type;

    union {
        struct {
            KeywordType keyword_type;
        } keyword_data;
        struct {
            BraceType brace_type;
        } brace_data;
        struct {
            NumberLexeme number;
        } number_data;
        struct {
            std::string_view identifier;
        } identifier_data;
        struct {
            OperatorType operator_type;
        } operator_data;
    };
};

struct LexicalAnalyzer {
    const char* text{};
    ParserPosition position{};
    Compiler* linked_compiler;
    std::vector<Lexeme> lexemes{};
    std::vector<std::string> compiled_files{};
    bool is_line_comment{};
    bool is_multiline_comment{};

    void error(const char* format, ...);

    LexicalAnalyzer(Compiler* compiler);

    std::vector<bonk::Lexeme> parse_file(const char* filename, const char* text);

    char next_char() const;

    void eat_char();

    bool parse_number_lexeme(Lexeme* target);

    int parse_digits_lexeme(int radix, long long int* integer_value, double* float_value);

    bool parse_identifier_lexeme(Lexeme* target);

    void make_operator_lexeme(Lexeme* lexeme, OperatorType type);

    void make_brace_lexeme(Lexeme* lexeme, BraceType type);

    KeywordType keyword_from_string(std::string_view string);

    bool add_compiled_file(const std::string& file_path);

    bool file_already_compiled(const std::string& file_path);

    bool next();
};

} // namespace bonk
