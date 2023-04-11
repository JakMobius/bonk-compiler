
#include "instruction_pool.hpp"
#include "hir.hpp"

char* bonk::InstructionPool::allocate_instruction(size_t size) {
    char* ptr = (char*)allocator.allocate(sizeof(void*) + size);

    if (!first_instruction) {
        first_instruction = ptr;
    }

    if (last_instruction) {
        *((void**)last_instruction) = ptr;
    }

    last_instruction = ptr;
    *((void**)ptr) = nullptr;

    return ptr + sizeof(void*);
}

void bonk::InstructionPool::deallocate_instructions() {
    for (void* ptr = first_instruction; ptr; ptr = *((void**)ptr)) {
        char* ptr2 = (char*)ptr;
        ptr2 += sizeof(void*);
        ((HIRInstruction*)ptr2)->~HIRInstruction();
    }
    allocator.clear();
    first_instruction = nullptr;
    last_instruction = nullptr;
}

bonk::InstructionPool::~InstructionPool() {
    deallocate_instructions();
}
