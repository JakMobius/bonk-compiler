
#include "ir.hpp"

char* bonk::IRInstructionPool::allocate_instruction(size_t size) {
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

void bonk::IRInstructionPool::deallocate_instructions() {
    for (void* ptr = first_instruction; ptr; ptr = *((void**)ptr)) {
        char* ptr2 = (char*)ptr;
        ptr2 += sizeof(void*);
        ((IRInstruction*)ptr2)->~IRInstruction();
    }
    allocator.clear();
    first_instruction = nullptr;
    last_instruction = nullptr;
}

bonk::IRInstructionPool::~IRInstructionPool() {
    deallocate_instructions();
}

void bonk::IRProgram::create_procedure() {
    procedures.emplace_back(*this);
}
