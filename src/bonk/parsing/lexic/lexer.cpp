
#include "lexer.hpp"
#include "../../compiler.hpp"

namespace bonk {

const char* BONK_OPERATOR_NAMES[] = {
    "@",    "+",    "-",  "*",   "/",  "+=",  "-=",   "*=",   "/=",   "=",
    "==",   "<",    ">",  "<=",  ">=", "!=",  "blok", "hive", "brek", "bowl",
    "bonk", "loop", "of", "and", "or", "not", "help", nullptr};

const char* BONK_KEYWORD_NAMES[] = {"buul", "shrt", "nubr", "long", "flot",
                                    "dabl", "strg", "many", "nothing", "null", nullptr};

const char* BONK_BRACE_NAMES[] = {"{", "}", "(", ")", "[", "]", nullptr};

const char* COMMENT_START = "dogo:";
const char* MULTILINE_COMMENT_START = "-dogo->";
const char* MULTILINE_COMMENT_END = "<-dogo-";

Lexer::Lexer(Compiler& compiler) : linked_compiler(compiler) {

    for (int i = 0; BONK_OPERATOR_NAMES[i]; i++) {
        special_words.emplace_back(BONK_OPERATOR_NAMES[i], LexerSpecialWordType::t_operator);
    }

    special_words.emplace_back(COMMENT_START, LexerSpecialWordType::t_line_comment);
    special_words.emplace_back(MULTILINE_COMMENT_START, LexerSpecialWordType::t_multiline_comment);
}

void Lexer::next() {
    if (!lexemes.empty()) {
        lexemes.back().end_position = current_position;
    }
    Lexeme next_lexeme = {};

    while (isspace(next_char())) {
        eat_char();
    }

    next_lexeme.start_position = current_position;

    switch (next_char()) {
    case ',':
        next_lexeme.type = LexemeType::l_comma;
        lexemes.push_back(next_lexeme);
        eat_char();
        return;
    case ':':
        next_lexeme.type = LexemeType::l_colon;
        lexemes.push_back(next_lexeme);
        eat_char();
        return;
    case ';':
        next_lexeme.type = LexemeType::l_semicolon;
        lexemes.push_back(next_lexeme);
        eat_char();
        return;
    case '\0':
        next_lexeme.type = LexemeType::l_eof;
        lexemes.push_back(next_lexeme);
        return;
    case '{':
    case '}':
    case '(':
    case ')':
    case '[':
    case ']':
        next_lexeme.type = LexemeType::l_brace;
        next_lexeme.data = BraceLexeme{BraceType(next_char())};
        eat_char();
        lexemes.push_back(next_lexeme);
        return;
    case '"':
    case '\'':
        parse_string_lexeme(&next_lexeme);
        lexemes.push_back(next_lexeme);
        return;
    }

    if (isdigit(next_char()) || next_char() == '.') {
        parse_number_lexeme(&next_lexeme);
        lexemes.push_back(next_lexeme);
        return;
    }

    // Try to match a special word
    int special_word = next_special_word();

    if (special_word != -1) {
        auto type = special_words[special_word].type;
        if (type == LexerSpecialWordType::t_operator) {
            next_lexeme.type = LexemeType::l_operator;
            next_lexeme.data = OperatorLexeme{OperatorType(special_word)};
            lexemes.push_back(next_lexeme);
        } else if (type == LexerSpecialWordType::t_line_comment) {
            parse_line_comment();
            return;
        } else if (type == LexerSpecialWordType::t_multiline_comment) {
            parse_multiline_comment();
            return;
        }
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
            next_lexeme.type = LexemeType::l_keyword;
            next_lexeme.data = KeywordLexeme{KeywordType(i)};
            lexemes.push_back(next_lexeme);
            return;
        }
    }

    // Otherwise it's an identifier
    next_lexeme.type = LexemeType::l_identifier;
    next_lexeme.data = IdentifierLexeme{word};
    lexemes.push_back(next_lexeme);
}

bool Lexer::parse_string_lexeme(Lexeme* target) {
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

std::vector<Lexeme> Lexer::parse_file(std::string_view filename, std::string_view source) {

    current_position.filename = filename;
    current_position.index = 0;
    current_position.ch = 1;
    current_position.line = 1;
    text = source;

    while (!linked_compiler.state) {
        next();
        if (!lexemes.empty() && lexemes.back().type == LexemeType::l_eof)
            break;
    }

    std::vector<Lexeme> result = std::move(lexemes);
    lexemes = {};

    return result;
}

char Lexer::next_char() const {
    return text[current_position.index];
}

void Lexer::eat_char() {
    char c = next_char();

    assert(c != '\0');

    current_position.ch++;
    current_position.index++;
    if (c == '\n') {
        current_position.line++;
        current_position.ch = 1;
    }
}

std::string_view Lexer::next_word() {
    int position = current_position.index;

    while (isalnum(next_char()) || next_char() == '_') {
        eat_char();
    }

    return text.substr(position, current_position.index - position);
}

int Lexer::next_special_word() {
    // Try to match any special word

    int position = current_position.index;
    int operators_left = special_words.size();
    bool could_be_identifier = true;

    for (auto&& i : special_words)
        i.matched = true;

    int matched_operator = -1;

    while (char c = text[position]) {
        int lexeme_position = position - current_position.index;

        // Invariant: special words don't contain numbers
        could_be_identifier = could_be_identifier && (isalnum(c) || c == '_');

        for (int i = 0; i < special_words.size(); i++) {
            if (!special_words[i].matched)
                continue;

            if (special_words[i].word[lexeme_position] == '\0') {
                operators_left--;
                matched_operator = i;
                special_words[i].matched = false;
                continue;
            }

            if (special_words[i].word[lexeme_position] != c) {
                operators_left--;
                special_words[i].matched = false;
                continue;
            }
        }

        if (operators_left == 0) {
            break;
        }

        position++;
    }

    if (matched_operator != -1) {
        // Check if the operator is not followed by an identifier character
        if (could_be_identifier && isalnum(text[position]) || text[position] == '_') {
            return -1;
        }

        int length = special_words[matched_operator].word.size();
        // Invariant: special words do not contain newlines
        current_position.index += length;
        current_position.ch += length;
        return matched_operator;
    }

    return -1;
}

void Lexer::parse_line_comment() {
    while (next_char() != '\n' && next_char() != '\0') {
        eat_char();
    }

    if (next_char() == '\n') {
        eat_char();
    }
}

void Lexer::parse_multiline_comment() {
    int match_length = 0;

    while (MULTILINE_COMMENT_END[match_length] != '\0') {
        if (next_char() == MULTILINE_COMMENT_END[match_length]) {
            match_length++;
        } else {
            match_length = 0;
        }

        eat_char();
    }
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

void NumberConstantContents::set_integer(long long int integer) {
    kind = NumberConstantKind::rather_integer;
    integer_value = integer;
    double_value = integer;
}

void NumberConstantContents::set_double(long double double_value) {
    kind = NumberConstantKind::rather_double;
    this->integer_value = double_value;
    this->double_value = double_value;
}

LexerSpecialWord::LexerSpecialWord(std::string_view word, LexerSpecialWordType type)
    : word(word), type(type) {
}
} // namespace bonk