
#include "parser_position.hpp"

namespace bonk {

ParserPosition* ParserPosition::clone() {
    auto* new_position = (ParserPosition*)calloc(1, sizeof(*this));

    new_position->index = index;
    new_position->line = line;
    new_position->ch = ch;

    new_position->filename = strdup(filename);

    return new_position;
}

const char* ParserPosition::to_string() {
    int length = snprintf(nullptr, 0, "%s:%lu:%lu", filename, line, ch);

    auto* buffer = (char*)calloc(length + 1, sizeof(char));

    snprintf(buffer, length + 1, "%s:%lu:%lu", filename, line, ch);

    return buffer;
}

} // namespace bonk