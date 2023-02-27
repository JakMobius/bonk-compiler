#pragma once

namespace bonk::x86_backend {

struct LinkerBlock;
struct LinkerConstant;

} // namespace bonk::x86_backend

#include <vector>

namespace bonk::x86_backend {

struct LinkerBlock {
    unsigned long long offset;

    std::vector<LinkerBlock*> nested_blocks;
    std::vector<LinkerConstant*> constants;
};

struct LinkerConstant {
    const char* constant_name;
    unsigned long long offset;
};

} // namespace bonk::x86_backend
