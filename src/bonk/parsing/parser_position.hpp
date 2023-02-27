#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace bonk {

struct ParserPosition {
    const char* filename;
    unsigned long line;
    unsigned long ch;
    unsigned long index;

    ParserPosition* clone();

    const char* to_string() const;
};

} // namespace bonk
