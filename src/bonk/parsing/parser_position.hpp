#pragma once

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

namespace bonk {

struct ParserPosition {
    const char* filename;
    unsigned long line;
    unsigned long ch;
    unsigned long index;

    ParserPosition* clone();

    const char* to_string();
};

} // namespace bonk
