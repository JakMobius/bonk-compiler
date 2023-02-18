#pragma once

namespace bonk::x86_backend {

struct linker_block;
struct linker_constant;

} // namespace bonk::x86_backend

#include <vector>

namespace bonk::x86_backend {

struct linker_block {
    unsigned long long offset;

    std::vector<linker_block*> nested_blocks;
    std::vector<linker_constant*> constants;
};

struct linker_constant {
    const char* constant_name;
    unsigned long long offset;
};

} // namespace bonk::x86_backend
