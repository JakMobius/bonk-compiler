#pragma once

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

namespace bonk {

struct parser_position {
    const char* filename;
    unsigned long line;
    unsigned long ch;
    unsigned long index;

    parser_position* clone();

    const char* to_string();
};

} // namespace bonk
