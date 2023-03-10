#pragma once

namespace bonk {

struct IRInstruction;
struct IRBaseBlock;
struct IRProcedure;
struct IRProgram;

} // namespace bonk

#include <list>
#include <vector>
#include "utils/linear_allocator.hpp"

namespace bonk {

typedef long long IRRegister;

struct IRInstruction {
    std::vector<IRRegister> read_registers;
    std::vector<IRRegister> write_registers;

    IRInstruction() = default;
    virtual ~IRInstruction() = default;
};

struct IRInstructionPool {
    LinearAllocator allocator{};

    void* last_instruction = nullptr;
    void* first_instruction = nullptr;

    template <typename T, typename... Args> T* instruction(Args&&... args) {
        char* ptr = allocate_instruction(sizeof(T));
        return new (ptr) T(std::forward<Args>(args)...);
    }

    char* allocate_instruction(size_t size);
    void deallocate_instructions();

    IRInstructionPool() = default;
    ~IRInstructionPool();
};

struct IRProgram {
    IRInstructionPool instruction_pool{};
    std::vector<IRProcedure> procedures{};

    // Proxy for IRInstructionPool::instruction
    template <typename T, typename... Args> T* instruction(Args&&... args) {
        return instruction_pool.instruction<T>(std::forward<Args>(args)...);
    }
    void create_procedure();
};

struct IRProcedure {
    IRProgram& program;
    std::vector<IRBaseBlock> base_blocks{};

    IRProcedure(IRProgram& program) : program(program) {
    }

    // Proxy for IRProgram::instruction
    template <typename T, typename... Args> T* instruction(Args&&... args) {
        return program.instruction<T>(std::forward<Args>(args)...);
    }

    void create_base_block() {
        base_blocks.emplace_back(*this);
    }
};

struct IRBaseBlock {
    IRProcedure& procedure;
    std::list<IRInstruction*> instructions{};
    std::vector<IRRegister> input_registers{};
    std::vector<IRRegister> output_registers{};

    IRBaseBlock(IRProcedure& procedure) : procedure(procedure) {
    }

    // Proxy for IRProcedure::instruction
    template <typename T, typename... Args> T* instruction(Args&&... args) {
        return procedure.instruction<T>(std::forward<Args>(args)...);
    }
};

} // namespace bonk