#pragma once

#include <memory>
#include <list>
#include <vector>
#include "utils/linear_allocator.hpp"

namespace bonk {

typedef long long IRRegister;

struct InstructionPool {
    LinearAllocator allocator{};

    void* last_instruction = nullptr;
    void* first_instruction = nullptr;

    template <typename T, typename... Args> T* instruction(Args&&... args) {
        char* ptr = allocate_instruction(sizeof(T));
        return new (ptr) T(std::forward<Args>(args)...);
    }

    char* allocate_instruction(size_t size);
    void deallocate_instructions();

    InstructionPool() = default;
    ~InstructionPool();
};

} // namespace bonk