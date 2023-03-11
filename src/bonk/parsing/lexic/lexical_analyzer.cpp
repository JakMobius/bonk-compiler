
#include "lexical_analyzer.hpp"
#include "../../compiler.hpp"

namespace bonk {

const char* BONK_OPERATOR_NAMES[] = {
    "@",    "+",    "-",    "*",    "/",  "+=",  "-=", "*=",   "/=",
    "=",    "==",   "<",    ">",    "<=", ">=",  "!=", "blok", "hive",
    "brek", "bowl", "bonk", "loop", "of", "and", "or", "not", "help", nullptr};

const char* BONK_KEYWORD_NAMES[] = {"flot", "nubr", "strg", "many", nullptr};

const char* BONK_BRACE_NAMES[] = {"{", "}", "(", ")", "[", "]", nullptr};

LexicalAnalyzer::LexicalAnalyzer(Compiler& compiler): linked_compiler(compiler) {

    int operator_count = 0;
    while (BONK_OPERATOR_NAMES[operator_count])
        operator_count++;
    operator_match.resize(operator_count);
}

void LexicalAnalyzer::next() {
    if (!lexemes.empty()) {
        lexemes.back().end_position = current_position;
    }
    Lexeme next_lexeme = {};
    next_lexeme.start_position = current_position;
    lexemes.push_back(next_lexeme);

    while (isspace(next_char())) {
        eat_char();
    }

    switch (next_char()) {
    case ',':
        lexemes.back().type = LexemeType::l_comma;
        eat_char();
        return;
    case ':':
        lexemes.back().type = LexemeType::l_colon;
        eat_char();
        return;
    case ';':
        lexemes.back().type = LexemeType::l_semicolon;
        eat_char();
        return;
    case '\0':
        lexemes.back().type = LexemeType::l_eof;
        return;
    case '{':
    case '}':
    case '(':
    case ')':
    case '[':
    case ']':
        lexemes.back().type = LexemeType::l_brace;
        lexemes.back().data = BraceLexeme{BraceType(next_char())};
        eat_char();
        return;
    case '"':
    case '\'':
        parse_string_lexeme(&lexemes.back());
        return;
    }

    if (isdigit(next_char()) || next_char() == '.') {
        parse_number_lexeme(&lexemes.back());
        return;
    }

    // Try to read an operator
    int operator_index = next_operator();

    if (operator_index != -1) {
        lexemes.back().type = LexemeType::l_operator;
        lexemes.back().data = OperatorLexeme{OperatorType(operator_index)};
        return;
    }

    // Read next word
    std::string_view word = next_word();
    if (word.empty()) {
        linked_compiler.error().at(current_position)
            << "unexpected character '" << next_char() << "'\n";
        return;
    }

    // Check if it's a keyword
    for (int i = 0; BONK_KEYWORD_NAMES[i]; i++) {
        if (word == BONK_KEYWORD_NAMES[i]) {
            lexemes.back().type = LexemeType::l_keyword;
            lexemes.back().data = KeywordLexeme{KeywordType(i)};
            return;
        }
    }

    // Otherwise it's an identifier
    lexemes.back().type = LexemeType::l_identifier;
    lexemes.back().data = IdentifierLexeme{word};
}

bool LexicalAnalyzer::parse_string_lexeme(Lexeme* target) {
    auto quote_type = QuoteType(next_char());
    target->type = LexemeType::l_string;

    std::stringstream stream;

    eat_char();

    while (next_char() != (char)quote_type) {
        if (next_char() == '\0') {
            linked_compiler.error().at(current_position)
                << "unexpected end of file while parsing string\n";
            return false;
        }

        if (next_char() == '\\') {
            eat_char();
            switch (next_char()) {
            case 'n':
                stream << '\n';
                break;
            case 't':
                stream << '\t';
                break;
            case 'r':
                stream << '\r';
                break;
            case '0':
                stream << '\0';
                break;
            case '\\':
                stream << '\\';
                break;
            case '"':
                stream << '"';
                break;
            case '\'':
                stream << '\'';
                break;
            default:
                linked_compiler.error().at(current_position)
                    << "unexpected escape sequence '\\" << next_char() << "'\n";
                return false;
            }
            eat_char();
            continue;
        }

        stream << next_char();
        eat_char();
    }

    eat_char();

    target->data = StringLexeme{quote_type, stream.str()};
    return true;
}

std::vector<Lexeme> LexicalAnalyzer::parse_file(const char* filename, std::string_view source) {

    filename = strdup(filename);

    current_position.filename = filename;
    current_position.index = 0;
    current_position.ch = 1;
    current_position.line = 1;
    text = source;

    while (!linked_compiler.state) {
        next();
        if (lexemes.back().type == LexemeType::l_eof)
            break;
    }

    std::vector<Lexeme> result = std::move(lexemes);
    lexemes = {};

    return result;
}

char LexicalAnalyzer::next_char() const {
    return text[current_position.index];
}

void LexicalAnalyzer::eat_char() {
    char c = next_char();

    assert(c != '\0');

    current_position.ch++;
    current_position.index++;
    if (c == '\n') {
        current_position.line++;
        current_position.ch = 1;
    }
}

std::string_view LexicalAnalyzer::next_word() {
    int position = current_position.index;

    while (isalnum(next_char()) || next_char() == '_' || isdigit(next_char())) {
        eat_char();
    }

    return text.substr(position, current_position.index - position);
}

int LexicalAnalyzer::next_operator() {
    // Try to match any operator listed in BONK_OPERATOR_NAMES

    int position = current_position.index;
    int operators_left = operator_match.size();

    for (auto&& i : operator_match) i = true;

    int matched_operator = -1;

    while (char c = text[position]) {
        int lexeme_position = position - current_position.index;

        for (int i = 0; i < operator_match.size(); i++) {
            if (!operator_match[i])
                continue;

            if (BONK_OPERATOR_NAMES[i][lexeme_position] == '\0') {
                operators_left--;
                matched_operator = i;
                operator_match[i] = false;
                continue;
            }

            if (BONK_OPERATOR_NAMES[i][lexeme_position] != c) {
                operators_left--;
                operator_match[i] = false;
                continue;
            }
        }

        if (operators_left == 0)
            break;

        position++;
    }

    if (matched_operator != -1) {
        int length = strlen(BONK_OPERATOR_NAMES[matched_operator]);
        // Invariant: operators do not contain newlines
        current_position.index += length;
        current_position.ch += length;
        return matched_operator;
    }

    return -1;
}

bool Lexeme::is(KeywordType keyword) const {
    return type == LexemeType::l_keyword && std::get<KeywordLexeme>(data).type == keyword;
}

bool Lexeme::is(OperatorType operator_type) const {
    return type == LexemeType::l_operator && std::get<OperatorLexeme>(data).type == operator_type;
}

bool Lexeme::is(BraceType brace_type) const {
    return type == LexemeType::l_brace && std::get<BraceLexeme>(data).type == brace_type;
}

bool Lexeme::is(LexemeType other_type) const {
    return type == other_type;
}

bool Lexeme::is_identifier(std::string_view exact) const {
    return type == LexemeType::l_identifier && std::get<IdentifierLexeme>(data).identifier == exact;
}

bool Lexeme::is_string(std::string_view exact) const {
    return type == LexemeType::l_string && std::get<StringLexeme>(data).string == exact;
}

} // namespace bonk