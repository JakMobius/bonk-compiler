#pragma once

// #define COLORIZER_DEBUG_ON

#ifdef COLORIZER_DEBUG_ON
#define COLORIZER_DEBUG(...) printf(__VA_ARGS__)
#else
#define COLORIZER_DEBUG(...)
#endif

namespace bonk::x86_backend {

struct abstract_register_usage;

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

extern const e_machine_register SYSTEM_V_OPTIMAL_REGISTERS[];

struct register_colorizer {

    // When instruction releases some register, it
    // gets stored here instead of being pushed to
    // unused registers stack. After instruction is
    // completed, registers_to_release array appends
    // to unused_registers_stack to prevent same
    // instruction to use the register which is released
    // by itself
    std::vector<e_machine_register> registers_to_release;

    std::vector<e_machine_register> registers_released_by_command;

    // Stack of unused registers
    std::vector<e_machine_register> unused_registers_stack;

    // Array of abstract registers associated with corresponding each real machine register
    abstract_register* machine_register_map = nullptr;

    register_colorize_context_state* state = nullptr;
    command_buffer* source = nullptr;

    std::vector<register_colorize_context_state> state_stack;
    std::unordered_map<jmp_label*, jmp_label*> label_map;

    command_buffer* output = nullptr;

    int frame_size = 0;
    std::vector<int> released_frame_cells{};

    register_colorizer(command_buffer* input);

    ~register_colorizer();

    static command_buffer* colorize(command_buffer* commands);

    void push_state(command_list* list);

    void pop_state();

    void colorize_command(asm_command* command, bool is_write);

    abstract_register get_colorized_register(asm_command* command, abstract_register reg,
                                             bool is_write);

    // Colorizes specific register, moves another registers
    void handle_register_use(asm_command* command, abstract_register reg, bool is_write);

    // Moves specified register to some another place (memory or register)
    void move_register(abstract_register reg, asm_command* cmd);

    bin_heap<abstract_register_usage>* get_register_usage_heap(abstract_register reg);

    void pop_nearest_register_usage(abstract_register reg, int max_scope = -1);

    long long retain_frame_cell();

    void release_frame_cell(long long cell);

    void run_colorizer();

    void save_register_location(abstract_register reg);

    void move_register_to_symbol(abstract_register reg);

    void move_register_to_stack(abstract_register reg, long long int frame_position);

    void move_register_to_register(abstract_register reg, e_machine_register target);

    void restore_register_from_symbol(abstract_register reg, e_machine_register target);

    void restore_register_from_stack(abstract_register reg, e_machine_register target);

    void restore_register(abstract_register reg, e_machine_register target);

    void exchange_registers(abstract_register reg_a, abstract_register reg_b);

    abstract_register least_used_register_for_command(asm_command* command, bool is_write);

    void give_location_for_register(abstract_register reg);

    void release_register_location(abstract_register reg, bool immediately);

    void destroy_register_if_unused(abstract_register reg);

    void release_register_immediately(e_machine_register reg, bool b);

    void release_register_after_command(e_machine_register reg);

    void flush_unused_registers();

    bool gather_unused_register(e_machine_register reg);

    bool is_unused_register(e_machine_register reg);

    e_machine_register effective_unused_register(asm_command* cmd, bool b);

    void transform_labels();

    bool command_uses_register(asm_command* cmd, e_machine_register reg, bool ignore_used);

    void replace_frame_commands();

    void handle_reg_preserve_command(reg_preserve_command* command);

    void handle_scope_pop_command(scope_pop_command* command);

    void handle_scope_command(scope_command* command);

    void handle_general_command(asm_command* command);

    void handle_locate_reg_reg_command(locate_reg_command* command);

    void handle_locate_reg_stack_command(locate_reg_command* command);

    void pop_scopes(int scopes);

    void locate_register_at_machine_register(abstract_register reg, e_machine_register loc,
                                             asm_command* command);

    void locate_register_at_stack(abstract_register reg, int frame_position);

    void pop_command_register_usages(asm_command* command, int current_scope = -1);

    void destroy_command_register_usages(asm_command* command);

    int frame_cell_for_register(abstract_register reg);

    void locate_register_at_symbol(abstract_register reg);

    void upload_symbol_registers();
};

} // namespace bonk::x86_backend
