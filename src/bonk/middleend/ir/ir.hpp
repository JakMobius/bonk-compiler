#pragma once

namespace bonk {

struct IRInstruction;
struct IRBaseBlock;
struct IRProcedure;
struct IRProgram;
struct IDTable;
struct SymbolTable;

} // namespace bonk

#include <memory>
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
    IDTable& id_table;
    SymbolTable& symbol_table;
    IRInstructionPool instruction_pool{};
    std::vector<std::unique_ptr<IRProcedure>> procedures{};

    IRProgram(IDTable& id_table, SymbolTable& symbol_table) : id_table(id_table), symbol_table(symbol_table) {
    }

    // Proxy for IRInstructionPool::instruction
    template <typename T, typename... Args> T* instruction(Args&&... args) {
        return instruction_pool.instruction<T>(std::forward<Args>(args)...);
    }
    void create_procedure();
};

struct IRProcedure {
    IRProgram& program;
    std::vector<std::unique_ptr<IRBaseBlock>> base_blocks{};

    IRProcedure(IRProgram& program) : program(program) {
    }

    // Proxy for IRProgram::instruction
    template <typename T, typename... Args> T* instruction(Args&&... args) {
        return program.instruction<T>(std::forward<Args>(args)...);
    }

    void create_base_block() {
        base_blocks.push_back(std::make_unique<IRBaseBlock>(*this));
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