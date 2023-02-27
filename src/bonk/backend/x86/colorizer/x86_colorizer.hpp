#pragma once

// #define COLORIZER_DEBUG_ON

#ifdef COLORIZER_DEBUG_ON
#define COLORIZER_DEBUG(...) printf(__VA_ARGS__)
#else
#define COLORIZER_DEBUG(...)
#endif

namespace bonk::x86_backend {

struct AbstractRegisterUsage;

}

#include <climits>
#include <set>
#include "../../../../utils/bin_heap.hpp"
#include "../instructions/x86_colorizer_align_stack.hpp"
#include "../instructions/x86_colorizer_locate_reg.hpp"
#include "../instructions/x86_colorizer_reg_preserve.hpp"
#include "../instructions/x86_colorizer_scope.hpp"
#include "../instructions/x86_colorizer_scope_pop.hpp"
#include "../instructions/x86_command.hpp"
#include "../instructions/x86_xchg.hpp"
#include "../x86_command_buffer.hpp"
#include "x86_colorizer_state.hpp"
#include "x86_colorizer_state_pop_context.hpp"

namespace bonk::x86_backend {

extern const MachineRegister SYSTEM_V_OPTIMAL_REGISTERS[];

struct RegisterColorizer {

    // When instruction releases some register, it
    // gets stored here instead of being pushed to
    // unused registers stack. After instruction is
    // completed, registers_to_release array appends
    // to unused_registers_stack to prevent same
    // instruction to use the register which is released
    // by itself
    std::vector<MachineRegister> registers_to_release;

    std::vector<MachineRegister> registers_released_by_command;

    // Stack of unused registers
    std::vector<MachineRegister> unused_registers_stack;

    // Array of abstract registers associated with corresponding each real machine register
    AbstractRegister* machine_register_map = nullptr;

    RegisterColorizeContextState* state = nullptr;
    CommandBuffer* source = nullptr;

    std::vector<RegisterColorizeContextState> state_stack;
    std::unordered_map<JmpLabel*, JmpLabel*> label_map;

    CommandBuffer* output = nullptr;

    int frame_size = 0;
    std::vector<int> released_frame_cells{};

    RegisterColorizer(CommandBuffer* input);

    ~RegisterColorizer();

    static CommandBuffer* colorize(CommandBuffer* commands);

    void push_state(CommandList* list);

    void pop_state();

    void colorize_command(AsmCommand* command, bool is_write);

    AbstractRegister get_colorized_register(AsmCommand* command, AbstractRegister reg,
                                             bool is_write);

    // Colorizes specific register, moves another registers
    void handle_register_use(AsmCommand* command, AbstractRegister reg, bool is_write);

    // Moves specified register to some another place (memory or register)
    void move_register(AbstractRegister reg, AsmCommand* cmd);

    bin_heap<AbstractRegisterUsage>* get_register_usage_heap(AbstractRegister reg);

    void pop_nearest_register_usage(AbstractRegister reg, int max_scope = -1);

    long long retain_frame_cell();

    void release_frame_cell(long long cell);

    void run_colorizer();

    void save_register_location(AbstractRegister reg);

    void move_register_to_symbol(AbstractRegister reg);

    void move_register_to_stack(AbstractRegister reg, long long int frame_position);

    void move_register_to_register(AbstractRegister reg, MachineRegister target);

    void restore_register_from_symbol(AbstractRegister reg, MachineRegister target);

    void restore_register_from_stack(AbstractRegister reg, MachineRegister target);

    void restore_register(AbstractRegister reg, MachineRegister target);

    void exchange_registers(AbstractRegister reg_a, AbstractRegister reg_b);

    AbstractRegister least_used_register_for_command(AsmCommand* command, bool is_write);

    void give_location_for_register(AbstractRegister reg);

    void release_register_location(AbstractRegister reg, bool immediately);

    void destroy_register_if_unused(AbstractRegister reg);

    void release_register_immediately(MachineRegister reg, bool b);

    void release_register_after_command(MachineRegister reg);

    void flush_unused_registers();

    bool gather_unused_register(MachineRegister reg);

    bool is_unused_register(MachineRegister reg);

    MachineRegister effective_unused_register(AsmCommand* cmd, bool b);

    void transform_labels();

    bool command_uses_register(AsmCommand* cmd, MachineRegister reg, bool ignore_used);

    void replace_frame_commands();

    void handle_reg_preserve_command(RegPreserveCommand* command);

    void handle_scope_pop_command(ScopePopCommand* command);

    void handle_scope_command(ScopeCommand* command);

    void handle_general_command(AsmCommand* command);

    void handle_locate_reg_reg_command(LocateRegCommand* command);

    void handle_locate_reg_stack_command(LocateRegCommand* command);

    void pop_scopes(int scopes);

    void locate_register_at_machine_register(AbstractRegister reg, MachineRegister loc,
                                             AsmCommand* command);

    void locate_register_at_stack(AbstractRegister reg, int frame_position);

    void pop_command_register_usages(AsmCommand* command, int current_scope = -1);

    void destroy_command_register_usages(AsmCommand* command);

    int frame_cell_for_register(AbstractRegister reg);

    void locate_register_at_symbol(AbstractRegister reg);

    void upload_symbol_registers();
};

} // namespace bonk::x86_backend
