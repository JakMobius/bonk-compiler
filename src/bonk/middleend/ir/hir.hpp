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
    location,
    phi_function
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
#include <vector>
#include "instruction_pool.hpp"

namespace bonk {

struct HIRProgram {
    IDTable& id_table;
    SymbolTable& symbol_table;
    InstructionPool instruction_pool{};
    std::vector<std::unique_ptr<HIRProcedure>> procedures{};

    HIRProgram(IDTable& id_table, SymbolTable& symbol_table)
        : id_table(id_table), symbol_table(symbol_table) {
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
    int used_registers = -1;

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
    int get_unused_register();
    void remove_killed_blocks();
};

struct HIRBaseBlock {
    int index = -1;
    HIRProcedure& procedure;
    std::list<HIRInstruction*> instructions{};
    std::vector<HIRBaseBlock*> predecessors{};
    std::vector<HIRBaseBlock*> successors{};

    HIRBaseBlock(HIRProcedure& procedure) : procedure(procedure) {
    }

    void kill() {
        index = -1;
        instructions.clear();
        predecessors.clear();
        successors.clear();
    }

    // Proxy for HIRProcedure::instruction
    template <typename T, typename... Args> T* instruction(Args&&... args) {
        return procedure.instruction<T>(std::forward<Args>(args)...);
    }

    void remove_killed_edges();
};

struct HIRInstruction {
    HIRInstructionType type = HIRInstructionType::unset;

    HIRInstruction(HIRInstructionType type);
    virtual ~HIRInstruction() = default;

    virtual int get_read_register_count() const {
        return 0;
    }
    virtual int get_write_register_count() const {
        return 0;
    }
    int get_operand_count() const {
        return get_read_register_count() + get_write_register_count();
    }

    const IRRegister& get_read_register(int index) const {
        return const_cast<HIRInstruction*>(this)->get_read_register(index);
    }
    const IRRegister& get_write_register(int index) const {
        return const_cast<HIRInstruction*>(this)->get_write_register(index, nullptr);
    }
    const IRRegister& get_operand(int index) const {
        return const_cast<HIRInstruction*>(this)->get_operand(index);
    }

    virtual IRRegister& get_read_register(int index) {
        assert(false);
    }
    virtual IRRegister& get_write_register(int index, HIRDataType* type) {
        assert(false);
    }

    IRRegister& get_operand(int index) {
        int read_count = get_read_register_count();
        if (index < read_count) {
            return get_read_register(index);
        } else {
            return get_write_register(index - read_count, nullptr);
        }
    }
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

    int get_write_register_count() const override {
        return 1;
    }
    IRRegister& get_write_register(int index, HIRDataType* type) override {
        if (type)
            *type = this->type;
        return target;
    }
};

struct HIRSymbolLoadInstruction : HIRInstruction {
    IRRegister target = 0;
    HIRDataType type = HIRDataType::unset;
    int symbol_id = -1;

    HIRSymbolLoadInstruction(IRRegister target, int symbol_id, HIRDataType type);

    int get_write_register_count() const override {
        return 1;
    }
    IRRegister& get_write_register(int index, HIRDataType* type) override {
        if (type)
            *type = this->type;
        return target;
    }
};

struct HIROperationInstruction : HIRInstruction {
    IRRegister target{};
    IRRegister left{};
    std::optional<IRRegister> right{};
    HIROperationType operation_type = HIROperationType::plus;
    HIRDataType operand_type = HIRDataType::unset;
    HIRDataType result_type = HIRDataType::unset;

    HIROperationInstruction();

    int get_read_register_count() const override {
        return right.has_value() ? 2 : 1;
    }
    IRRegister& get_read_register(int index) override {
        return index == 0 ? left : right.value();
    }
    int get_write_register_count() const override {
        return 1;
    }
    IRRegister& get_write_register(int index, HIRDataType* type) override {
        if (type)
            *type = result_type;
        return target;
    }

    HIROperationInstruction& set_assign(IRRegister target, IRRegister left, HIRDataType type);
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

    int get_read_register_count() const override {
        return 1;
    }
    IRRegister& get_read_register(int index) override {
        return condition;
    }
};

struct HIRCallInstruction : HIRInstruction {
    HIRDataType return_type = HIRDataType::unset;
    std::optional<IRRegister> return_value = std::nullopt;

    int procedure_label_id = -1;

    HIRCallInstruction();

    int get_write_register_count() const override {
        return return_value.has_value() ? 1 : 0;
    }
    IRRegister& get_write_register(int index, HIRDataType* type) override {
        if (type)
            *type = return_type;
        return return_value.value();
    }
};

struct HIRReturnInstruction : HIRInstruction {
    HIRDataType return_type = HIRDataType::unset;
    std::optional<IRRegister> return_value = std::nullopt;

    HIRReturnInstruction(IRRegister return_value);
    HIRReturnInstruction();

    int get_read_register_count() const override {
        return return_value.has_value() ? 1 : 0;
    }
    IRRegister& get_read_register(int index) override {
        return return_value.value();
    }
};

struct HIRParameterInstruction : HIRInstruction {
    HIRDataType type = HIRDataType::unset;
    IRRegister parameter = 0;

    HIRParameterInstruction();

    int get_read_register_count() const override {
        return 1;
    }
    IRRegister& get_read_register(int index) override {
        return parameter;
    }
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
    HIRMemoryLoadInstruction(IRRegister target, IRRegister address, HIRDataType type);

    int get_write_register_count() const override {
        return 1;
    }
    IRRegister& get_write_register(int index, HIRDataType* type) override {
        if (type)
            *type = this->type;
        return target;
    }
    int get_read_register_count() const override {
        return 1;
    }
    IRRegister& get_read_register(int index) override {
        return address;
    }
};

struct HIRMemoryStoreInstruction : HIRInstruction {
    IRRegister address = 0;
    IRRegister value = 0;
    HIRDataType type = HIRDataType::unset;

    HIRMemoryStoreInstruction();

    int get_read_register_count() const override {
        return 2;
    }
    IRRegister& get_read_register(int index) override {
        return index == 0 ? address : value;
    }
};

struct HIRIncRefCounterInstruction : HIRInstruction {
    IRRegister address = 0;

    HIRIncRefCounterInstruction();

    int get_read_register_count() const override {
        return 1;
    }
    IRRegister& get_read_register(int index) override {
        return address;
    }
};

struct HIRDecRefCounterInstruction : HIRInstruction {
    IRRegister address = 0;
    TreeNodeHiveDefinition* hive_definition = nullptr;

    HIRDecRefCounterInstruction();

    int get_read_register_count() const override {
        return 1;
    }
    IRRegister& get_read_register(int index) override {
        return address;
    }
};

struct HIRFileInstruction : HIRInstruction {
    std::string_view file{};

    HIRFileInstruction();
};

struct HIRLocationInstruction : HIRInstruction {
    unsigned int line = 0;
    unsigned int column = 0;

    HIRLocationInstruction();
};

struct HIRPhiFunctionInstruction : HIRInstruction {
    IRRegister target = 0;
    HIRDataType type = HIRDataType::unset;
    std::vector<IRRegister> sources{};

    HIRPhiFunctionInstruction();

    int get_write_register_count() const override {
        return 1;
    }
    IRRegister& get_write_register(int index, HIRDataType* type) override {
        if (type)
            *type = this->type;
        return target;
    }
    int get_read_register_count() const override {
        return sources.size();
    }
    IRRegister& get_read_register(int index) override {
        return sources[index];
    }
};

} // namespace bonk