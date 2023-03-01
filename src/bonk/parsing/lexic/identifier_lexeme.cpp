
/*
 * Identifier lexeme
 * $IDENTIFIER = [a-zA-Z][a-zA-Z0-9]+
 */

#include "identifier_lexeme.hpp"

namespace bonk {

bool LexicalAnalyzer::parse_identifier_lexeme(Lexeme* target) {
    const char* name_start = text + position.index;

    ParserPosition identifier_position = position;

    while (true) {
        char c = next_char();
        if (isdigit(c)) {
            if (position.index == identifier_position.index) {
                break;
            }
        } else if (!isalpha(c) && c != '_')
            break;

        eat_char();
    }

    if (identifier_position.index == position.index)
        return false;

    KeywordType keyword = keyword_from_string(
        std::string_view(name_start, position.index - identifier_position.index));
    if (keyword != (KeywordType)-1) {
        target->type = BONK_LEXEME_KEYWORD;
        target->keyword_data.keyword_type = keyword;
        return true;
    }

    unsigned long length = position.index - identifier_position.index;

    target->type = BONK_LEXEME_IDENTIFIER;
    target->identifier_data.identifier = std::string_view(name_start, length);
    return true;
}

KeywordType LexicalAnalyzer::keyword_from_string(std::string_view string) {
    for (int keyword = 0; BONK_KEYWORD_NAMES[(KeywordType)keyword]; keyword++) {
        if (strncmp(string.data(), BONK_KEYWORD_NAMES[keyword], string.size()) == 0) {
            if (BONK_KEYWORD_NAMES[keyword][string.size()] == '\0')
                return (KeywordType)keyword;
        }
    }
    return (KeywordType)-1;
}

} // namespace bonk