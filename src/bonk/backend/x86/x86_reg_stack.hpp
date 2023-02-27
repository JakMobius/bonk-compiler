#pragma once

namespace bonk::x86_backend {

struct RegStack;
struct CommandList;
struct RegStackEntry;

} // namespace bonk::x86_backend

#include <cstdlib>
#include "instructions/x86_cmp.hpp"
#include "instructions/x86_conditional_instructions.hpp"
#include "instructions/x86_logical_instructions.hpp"
#include "instructions/x86_test.hpp"
#include "x86_abstract_register.hpp"

namespace bonk::x86_backend {

struct RegStackEntry {
    AbstractRegister reg;
    bool is_logical;
};

struct RegStack {

    std::vector<RegStackEntry> stack{};

    CommandList* list;

    RegStack(CommandList* list);

    void push_imm64(uint64_t value);

    void push_reg64(AbstractRegister reg);

    void pop(AbstractRegister reg);

    void add();

    void sub();

    void mul();

    void div();

    RegStackEntry get_head_register();

    AbstractRegister get_head_register_number();

    AbstractRegister get_head_register_logical();

    bool push_next_register(bool is_logical);

    void decrease_stack_size();

    void logical_and();

    void logical_or();

    void set_head_type(bool is_logical);

    void equals();

    void less_than();

    void less_or_equal_than();

    void greater_than();

    void greater_or_equal_than();

    void not_equal();

    void convert_logical_to_number(AbstractRegister reg);

    void convert_number_to_logical(AbstractRegister reg);

    void compare();

    void test();

    AbstractRegister push_placeholder(bool is_logical);

    void pop_logical(AbstractRegister reg);

    void write_head(AbstractRegister reg);
};

} // namespace bonk::x86_backend
