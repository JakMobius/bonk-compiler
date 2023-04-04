#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ostream>

namespace bonk {

struct ParserPosition {
    std::string_view filename;
    unsigned int line;
    unsigned int ch;
    unsigned int index;

    // To print position
    friend std::ostream& operator<<(std::ostream& stream, const ParserPosition& position) {
        stream << position.filename << ":" << position.line << ":" << position.ch;
        return stream;
    }
};

} // namespace bonk
