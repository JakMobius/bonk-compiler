
/*
 * Identifier lexeme
 * $IDENTIFIER = [a-zA-Z][a-zA-Z0-9]+
 */

#include "identifier_lexeme.hpp"

namespace bonk {

bool Lexer::parse_identifier_lexeme(Lexeme* target) {
    const char* name_start = text.data() + current_position.index;

    ParserPosition identifier_position = current_position;

    while (true) {
        char c = next_char();
        if (isdigit(c)) {
            if (current_position.index == identifier_position.index) {
                break;
            }
        } else if (!isalpha(c) && c != '_')
            break;

        eat_char();
    }

    if (identifier_position.index == current_position.index)
        return false;

    KeywordType keyword = keyword_from_string(
        std::string_view(name_start, current_position.index - identifier_position.index));
    if (keyword != (KeywordType)-1) {
        target->type = LexemeType::l_keyword;
        target->data = KeywordLexeme{keyword};
        return true;
    }

    unsigned long length = current_position.index - identifier_position.index;

    target->type = LexemeType::l_identifier;
    target->data = IdentifierLexeme{std::string_view(name_start, length)};
    return true;
}

KeywordType Lexer::keyword_from_string(std::string_view string) {
    for (int keyword = 0; BONK_KEYWORD_NAMES[keyword]; keyword++) {
        if (strncmp(string.data(), BONK_KEYWORD_NAMES[keyword], string.size()) == 0) {
            if (BONK_KEYWORD_NAMES[keyword][string.size()] == '\0')
                return (KeywordType)keyword;
        }
    }
    return (KeywordType)-1;
}

} // namespace bonk