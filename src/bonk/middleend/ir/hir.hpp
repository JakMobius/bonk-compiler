#pragma once

namespace bonk {

struct HIRBaseBlock;
struct HIRProcedure;
struct HIRProgram;
struct HIRInstruction;
struct TreeNodeHiveDefinition;
struct IDTable;
struct SymbolTable;
struct HIRProcedureParameter;

enum class HIRInstructionType {
    unset,
    label,
    constant_load,
    symbol_load,
    operation,
    jump,
    jump_nz,
    call,
    return_op,
    parameter,
    memory_load,
    memory_store,
    inc_ref_counter,
    dec_ref_counter,
    file,
    location
};

enum class HIROperationType {
    plus,
    minus,
    multiply,
    divide,
    assign,
    and_op,
    or_op,
    xor_op,
    not_op,
    equal,
    not_equal,
    less,
    less_equal,
    greater,
    greater_equal
};

enum class HIRDataType {
    unset,
    byte,
    hword,
    word,
    dword,
    float32,
    float64,
};

} // namespace bonk

#include <optional>
#include <string>
#include "instruction_pool.hpp"

namespace bonk {

struct HIRProgram {
    IDTable& id_table;
    SymbolTable& symbol_table;
    InstructionPool instruction_pool{};
    std::vector<std::unique_ptr<HIRProcedure>> procedures{};

    HIRProgram(IDTable& id_table, SymbolTable& symbol_table) : id_table(id_table), symbol_table(symbol_table) {
    }

    // Proxy for IRInstructionPool::instruction
    template <typename T, typename... Args> T* instruction(Args&&... args) {
        return instruction_pool.instruction<T>(std::forward<Args>(args)...);
    }
    void create_procedure();
};

struct HIRProcedure {
    HIRProgram& program;
    std::vector<std::unique_ptr<HIRBaseBlock>> base_blocks{};

    int procedure_id = -1;
    int start_block_index = -1;
    int end_block_index = -1;

    std::vector<HIRProcedureParameter> parameters;
    HIRDataType return_type = HIRDataType::unset;
    bool is_external = false;

    HIRProcedure(HIRProgram& program) : program(program) {
    }

    void add_control_flow_edge(HIRBaseBlock* from, HIRBaseBlock* to);

    // Proxy for HIRProgram::instruction
    template <typename T, typename... Args> T* instruction(Args&&... args) {
        return program.instruction<T>(std::forward<Args>(args)...);
    }

    void create_base_block();
};

struct HIRBaseBlock {
    int index = -1;
    HIRProcedure& procedure;
    std::list<HIRInstruction*> instructions{};
    std::vector<HIRBaseBlock*> predecessors{};
    std::vector<HIRBaseBlock*> successors{};

    HIRBaseBlock(HIRProcedure& procedure) : procedure(procedure) {
    }

    // Proxy for HIRProcedure::instruction
    template <typename T, typename... Args> T* instruction(Args&&... args) {
        return procedure.instruction<T>(std::forward<Args>(args)...);
    }
};

struct HIRInstruction {
    HIRInstructionType type = HIRInstructionType::unset;

    HIRInstruction(HIRInstructionType type);
    virtual ~HIRInstruction() = default;
};

struct HIRLabelInstruction : HIRInstruction {
    int label_id = -1;

    HIRLabelInstruction(int label_id);
};

struct HIRConstantLoadInstruction : HIRInstruction {
    IRRegister target = 0;
    HIRDataType type = HIRDataType::unset;
    long long constant = 0;

    HIRConstantLoadInstruction(IRRegister target, long long constant, HIRDataType type);
    HIRConstantLoadInstruction(IRRegister target, int64_t constant);
    HIRConstantLoadInstruction(IRRegister target, int32_t constant);
    HIRConstantLoadInstruction(IRRegister target, int16_t constant);
    HIRConstantLoadInstruction(IRRegister target, int8_t constant);
    HIRConstantLoadInstruction(IRRegister target, float constant);
    HIRConstantLoadInstruction(IRRegister target, double constant);
};

struct HIRSymbolLoadInstruction : HIRInstruction {
    IRRegister target = 0;
    HIRDataType type = HIRDataType::unset;
    int symbol_id = -1;

    HIRSymbolLoadInstruction(IRRegister target, int symbol_id, HIRDataType type);
};

struct HIROperationInstruction : HIRInstruction {
    IRRegister target{};
    IRRegister left{};
    std::optional<IRRegister> right{};
    HIROperationType operation_type = HIROperationType::plus;
    HIRDataType operand_type = HIRDataType::unset;
    HIRDataType result_type = HIRDataType::unset;

    HIROperationInstruction();
};

struct HIRJumpInstruction : HIRInstruction {
    int label_id = -1;

    HIRJumpInstruction();
    HIRJumpInstruction(int label_id);
};

struct HIRJumpNZInstruction : HIRInstruction {
    IRRegister condition = 0;
    int nz_label = -1;
    int z_label = -1;

    HIRJumpNZInstruction();
    HIRJumpNZInstruction(IRRegister condition, int nz_label, int z_label);
};

struct HIRCallInstruction : HIRInstruction {
    HIRDataType return_type = HIRDataType::unset;
    std::optional<IRRegister> return_value = std::nullopt;

    int procedure_label_id = -1;

    HIRCallInstruction();
};

struct HIRReturnInstruction : HIRInstruction {
    HIRDataType return_type = HIRDataType::unset;
    std::optional<IRRegister> return_value = std::nullopt;

    HIRReturnInstruction(IRRegister return_value);
    HIRReturnInstruction();
};

struct HIRParameterInstruction : HIRInstruction {
    HIRDataType type = HIRDataType::unset;
    IRRegister parameter = 0;

    HIRParameterInstruction();
};

struct HIRProcedureParameter {
    HIRDataType type = HIRDataType::unset;
    IRRegister register_id = 0;
};

struct HIRMemoryLoadInstruction : HIRInstruction {
    IRRegister target = 0;
    IRRegister address = 0;
    HIRDataType type = HIRDataType::unset;

    HIRMemoryLoadInstruction();
};

struct HIRMemoryStoreInstruction : HIRInstruction {
    IRRegister address = 0;
    IRRegister value = 0;
    HIRDataType type = HIRDataType::unset;

    HIRMemoryStoreInstruction();
};

struct HIRIncRefCounterInstruction : HIRInstruction {
    IRRegister address = 0;

    HIRIncRefCounterInstruction();
};

struct HIRDecRefCounterInstruction : HIRInstruction {
    IRRegister address = 0;
    TreeNodeHiveDefinition* hive_definition = nullptr;

    HIRDecRefCounterInstruction();
};

struct HIRFileInstruction: HIRInstruction {
    std::string_view file {};

    HIRFileInstruction();
};

struct HIRLocationInstruction : HIRInstruction {
    unsigned int line = 0;
    unsigned int column = 0;

    HIRLocationInstruction();
};

} // namespace bonk