
#pragma once

namespace bonk {

struct Lexeme;
struct LexicalAnalyzer;
struct ParserPosition;
struct Compiler;

enum class LexemeType {
    l_keyword,
    l_identifier,
    l_number,
    l_semicolon,
    l_colon,
    l_string,
    l_operator,
    l_brace,
    l_comma,
    l_eof
};

enum class KeywordType {
    k_flot,
    k_nubr,
    k_strg,
    k_many
};

enum class OperatorType {
    o_call,
    o_plus,
    o_minus,
    o_multiply,
    o_divide,
    o_plus_assign,
    o_minus_assign,
    o_multiply_assign,
    o_divide_assign,
    o_assign,
    o_equal,
    o_less,
    o_greater,
    o_less_equal,
    o_greater_equal,
    o_not_equal,
    o_blok,
    o_hive,
    o_brek,
    o_bowl,
    o_bonk,
    o_loop,
    o_of,
    o_and,
    o_or,
    o_help,
    o_invalid
};

enum class BraceType {
    l_cb = '{',
    r_cb = '}',
    l_rb = '(',
    r_rb = ')',
    l_sb = '[',
    r_sb = ']'
};

enum class QuoteType {
    q_single = '\'',
    q_double = '"'
};

extern const char* BONK_OPERATOR_NAMES[];
extern const char* BONK_KEYWORD_NAMES[];
extern const char* BONK_BRACE_NAMES[];

} // namespace bonk

#include <cctype>
#include <variant>
#include <vector>
#include "../parser_position.hpp"
#include "bonk/tree/ast.hpp"
#include "identifier_lexeme.hpp"
#include "number_lexeme.hpp"

namespace bonk {

struct NumberLexeme {
    long long int integer_value;
    long double double_value;
};

struct KeywordLexeme {
    KeywordType type;
};

struct OperatorLexeme {
    OperatorType type;
};

struct BraceLexeme {
    BraceType type;
};

struct IdentifierLexeme {
    std::string_view identifier;
};

struct StringLexeme {
    QuoteType type;
    std::string string;
};

struct Lexeme {
    ParserPosition start_position;
    ParserPosition end_position;
    LexemeType type;

    std::variant<
        KeywordLexeme,
        IdentifierLexeme,
        NumberLexeme,
        OperatorLexeme,
        BraceLexeme,
        StringLexeme
    > data;

    bool is(KeywordType keyword);
    bool is(OperatorType operator_type);
    bool is(BraceType brace_type);
    bool is_number();
    bool is_identifier();
    bool is_identifier(std::string_view exact);
    bool is_string();
    bool is_string(std::string_view exact);
};

enum class OperatorMatch {
    not_matched,
    maybe_matched,
    matched
};

struct LexicalAnalyzer {
    std::string_view text{};
    Compiler* linked_compiler;
    ParserPosition current_position{};
    std::vector<Lexeme> lexemes{};
    std::vector<OperatorMatch> operator_match{};

    LexicalAnalyzer(Compiler* compiler);

    std::vector<bonk::Lexeme> parse_file(const char* filename, std::string_view text);

    void next();
    char next_char() const;
    void eat_char();
    int next_operator();
    std::string_view next_word();

    int parse_digits_lexeme(int radix, long long int* integer_value, double* float_value);

    bool parse_identifier_lexeme(Lexeme* target);
    bool parse_number_lexeme(Lexeme* target);
    bool parse_string_lexeme(Lexeme* target);

    KeywordType keyword_from_string(std::string_view string);
};

} // namespace bonk
