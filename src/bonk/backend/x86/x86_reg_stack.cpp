
#include "x86_reg_stack.hpp"
#include "instructions/x86_add.hpp"
#include "instructions/x86_idiv.hpp"
#include "instructions/x86_imul.hpp"
#include "instructions/x86_mov.hpp"
#include "instructions/x86_sub.hpp"
#include "instructions/x86_xor.hpp"
#include "x86_register_extensions.hpp"

namespace bonk::x86_backend {

reg_stack::reg_stack(command_list* list) {
    this->list = list;
}

void reg_stack::push_imm64(uint64_t value) {
    push_next_register(false);
    list->insert_tail(
        new mov_command(command_parameter::create_register_64(get_head_register().reg),
                        command_parameter::create_imm32(value)));
}

abstract_register reg_stack::push_placeholder(bool is_logical) {
    push_next_register(is_logical);
    return get_head_register().reg;
}

void reg_stack::push_reg64(abstract_register reg) {
    push_next_register(false);
    list->insert_tail(
        new mov_command(command_parameter::create_register_64(get_head_register().reg),
                        command_parameter::create_register_64(reg)));
}

void reg_stack::logical_and() {
    abstract_register source = get_head_register_logical();
    decrease_stack_size();
    abstract_register destination = get_head_register_logical();

    list->insert_tail(new and_command(command_parameter::create_register_8(destination),
                                      command_parameter::create_register_8(source)));

    set_head_type(true);
}

void reg_stack::logical_or() {
    abstract_register source = get_head_register_logical();
    decrease_stack_size();
    abstract_register destination = get_head_register_logical();

    list->insert_tail(new or_command(command_parameter::create_register_8(destination),
                                     command_parameter::create_register_8(source)));

    set_head_type(true);
}

void reg_stack::test() {
    reg_stack_entry entry = get_head_register();

    if (entry.is_logical) {
        list->insert_tail(new test_command(command_parameter::create_register_8(entry.reg),
                                           command_parameter::create_register_8(entry.reg)));
    } else {
        list->insert_tail(new test_command(command_parameter::create_register_64(entry.reg),
                                           command_parameter::create_register_64(entry.reg)));
    }

    decrease_stack_size();
}

void reg_stack::compare() {
    reg_stack_entry right = get_head_register();
    decrease_stack_size();
    reg_stack_entry left = get_head_register();
    decrease_stack_size();

    if (right.is_logical != left.is_logical) {
        if (right.is_logical)
            convert_logical_to_number(right.reg);
        else
            convert_logical_to_number(left.reg);
    }
    if (right.is_logical) {
        list->insert_tail(new cmp_command(command_parameter::create_register_8(left.reg),
                                          command_parameter::create_register_8(right.reg)));
    } else {
        list->insert_tail(new cmp_command(command_parameter::create_register_64(left.reg),
                                          command_parameter::create_register_64(right.reg)));
    }
}

void reg_stack::equals() {
    compare();
    push_next_register(true);
    list->insert_tail(new cset_command(get_head_register().reg, COMMAND_SETE));
}

void reg_stack::less_than() {
    compare();
    push_next_register(true);
    list->insert_tail(new cset_command(get_head_register().reg, COMMAND_SETL));
}

void reg_stack::less_or_equal_than() {
    compare();
    push_next_register(true);
    list->insert_tail(new cset_command(get_head_register().reg, COMMAND_SETNG));
}

void reg_stack::greater_than() {
    compare();
    push_next_register(true);
    list->insert_tail(new cset_command(get_head_register().reg, COMMAND_SETG));
}

void reg_stack::greater_or_equal_than() {
    compare();
    push_next_register(true);
    list->insert_tail(new cset_command(get_head_register().reg, COMMAND_SETNL));
}

void reg_stack::not_equal() {
    compare();
    push_next_register(true);
    list->insert_tail(new cset_command(get_head_register().reg, COMMAND_SETNE));
}

void reg_stack::add() {
    abstract_register source = get_head_register_number();
    decrease_stack_size();
    abstract_register destination = get_head_register_number();

    list->insert_tail(new add_command(command_parameter::create_register_64(destination),
                                      command_parameter::create_register_64(source)));

    set_head_type(false);
}

void reg_stack::sub() {
    abstract_register source = get_head_register_number();
    decrease_stack_size();
    abstract_register destination = get_head_register_number();
    list->insert_tail(new sub_command(command_parameter::create_register_64(destination),
                                      command_parameter::create_register_64(source)));

    set_head_type(false);
}

void reg_stack::mul() {
    abstract_register source = get_head_register_number();
    decrease_stack_size();
    abstract_register destination = get_head_register_number();
    list->insert_tail(new imul_command(destination, source));

    set_head_type(false);
}

void reg_stack::div() {
    abstract_register source = get_head_register_number();
    decrease_stack_size();
    abstract_register destination = get_head_register_number();

    abstract_register rdx_handle =
        list->parent_buffer->descriptors.next_constrained_register(rdx, list);
    abstract_register rax_handle =
        list->parent_buffer->descriptors.next_constrained_register(rax, list);

    list->insert_tail(new xor_command(command_parameter::create_register_64(rdx_handle),
                                      command_parameter::create_register_64(rdx_handle)));
    list->insert_tail(new mov_command(command_parameter::create_register_64(rax_handle),
                                      command_parameter::create_register_64(destination)));
    list->insert_tail(new idiv_command(source, rax_handle, rdx_handle));
    list->insert_tail(new mov_command(command_parameter::create_register_64(destination),
                                      command_parameter::create_register_64(rax_handle)));

    set_head_type(false);
}

reg_stack_entry reg_stack::get_head_register() {
    return stack[stack.size() - 1];
}

void reg_stack::set_head_type(bool is_logical) {
    stack[stack.size() - 1].is_logical = is_logical;
}

bool reg_stack::push_next_register(bool is_logical) {
    abstract_register reg = list->parent_buffer->descriptors.next_register(list);

    stack.push_back({reg, is_logical});
    return true;
}

void reg_stack::decrease_stack_size() {
    stack.pop_back();
}

void reg_stack::write_head(abstract_register reg) {
    list->insert_tail(
        new mov_command(command_parameter::create_register_64(reg),
                        command_parameter::create_register_64(get_head_register().reg)));
}

void reg_stack::pop(abstract_register reg) {
    write_head(reg);
    decrease_stack_size();
}

void reg_stack::convert_logical_to_number(abstract_register reg) {
    list->insert_tail(new movzx_command(command_parameter::create_register_64(reg),
                                        command_parameter::create_register_8(reg)));
}

void reg_stack::convert_number_to_logical(abstract_register reg) {
    list->insert_tail(new test_command(command_parameter::create_register_64(reg),
                                       command_parameter::create_register_64(reg)));
    list->insert_tail(new cset_command(reg, COMMAND_SETNE));
}

abstract_register reg_stack::get_head_register_number() {
    auto head = get_head_register();
    if (head.is_logical)
        convert_logical_to_number(head.reg);
    return head.reg;
}

abstract_register reg_stack::get_head_register_logical() {
    auto head = get_head_register();
    if (!head.is_logical)
        convert_number_to_logical(head.reg);
    return head.reg;
}

} // namespace bonk::x86_backend