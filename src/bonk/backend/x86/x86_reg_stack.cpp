
#include "x86_reg_stack.hpp"
#include "instructions/x86_add.hpp"
#include "instructions/x86_idiv.hpp"
#include "instructions/x86_imul.hpp"
#include "instructions/x86_mov.hpp"
#include "instructions/x86_sub.hpp"
#include "instructions/x86_xor.hpp"
#include "x86_register_extensions.hpp"

namespace bonk::x86_backend {

RegStack::RegStack(CommandList* list) {
    this->list = list;
}

void RegStack::push_imm64(uint64_t value) {
    push_next_register(false);
    list->insert_tail(
        new MovCommand(CommandParameter::create_register_64(get_head_register().reg),
                                      CommandParameter::create_imm32(value)));
}

AbstractRegister RegStack::push_placeholder(bool is_logical) {
    push_next_register(is_logical);
    return get_head_register().reg;
}

void RegStack::push_reg64(AbstractRegister reg) {
    push_next_register(false);
    list->insert_tail(
        new MovCommand(CommandParameter::create_register_64(get_head_register().reg),
                                      CommandParameter::create_register_64(reg)));
}

void RegStack::logical_and() {
    AbstractRegister source = get_head_register_logical();
    decrease_stack_size();
    AbstractRegister destination = get_head_register_logical();

    list->insert_tail(new AndCommand(CommandParameter::create_register_8(destination),
                                      CommandParameter::create_register_8(source)));

    set_head_type(true);
}

void RegStack::logical_or() {
    AbstractRegister source = get_head_register_logical();
    decrease_stack_size();
    AbstractRegister destination = get_head_register_logical();

    list->insert_tail(new OrCommand(CommandParameter::create_register_8(destination),
                                     CommandParameter::create_register_8(source)));

    set_head_type(true);
}

void RegStack::test() {
    RegStackEntry entry = get_head_register();

    if (entry.is_logical) {
        list->insert_tail(new TestCommand(CommandParameter::create_register_8(entry.reg),
                                           CommandParameter::create_register_8(entry.reg)));
    } else {
        list->insert_tail(new TestCommand(CommandParameter::create_register_64(entry.reg),
                                           CommandParameter::create_register_64(entry.reg)));
    }

    decrease_stack_size();
}

void RegStack::compare() {
    RegStackEntry right = get_head_register();
    decrease_stack_size();
    RegStackEntry left = get_head_register();
    decrease_stack_size();

    if (right.is_logical != left.is_logical) {
        if (right.is_logical)
            convert_logical_to_number(right.reg);
        else
            convert_logical_to_number(left.reg);
    }
    if (right.is_logical) {
        list->insert_tail(new CmpCommand(CommandParameter::create_register_8(left.reg),
                                          CommandParameter::create_register_8(right.reg)));
    } else {
        list->insert_tail(new CmpCommand(CommandParameter::create_register_64(left.reg),
                                          CommandParameter::create_register_64(right.reg)));
    }
}

void RegStack::equals() {
    compare();
    push_next_register(true);
    list->insert_tail(new CSetCommand(get_head_register().reg, COMMAND_SETE));
}

void RegStack::less_than() {
    compare();
    push_next_register(true);
    list->insert_tail(new CSetCommand(get_head_register().reg, COMMAND_SETL));
}

void RegStack::less_or_equal_than() {
    compare();
    push_next_register(true);
    list->insert_tail(new CSetCommand(get_head_register().reg, COMMAND_SETNG));
}

void RegStack::greater_than() {
    compare();
    push_next_register(true);
    list->insert_tail(new CSetCommand(get_head_register().reg, COMMAND_SETG));
}

void RegStack::greater_or_equal_than() {
    compare();
    push_next_register(true);
    list->insert_tail(new CSetCommand(get_head_register().reg, COMMAND_SETNL));
}

void RegStack::not_equal() {
    compare();
    push_next_register(true);
    list->insert_tail(new CSetCommand(get_head_register().reg, COMMAND_SETNE));
}

void RegStack::add() {
    AbstractRegister source = get_head_register_number();
    decrease_stack_size();
    AbstractRegister destination = get_head_register_number();

    list->insert_tail(new AddCommand(CommandParameter::create_register_64(destination),
                                      CommandParameter::create_register_64(source)));

    set_head_type(false);
}

void RegStack::sub() {
    AbstractRegister source = get_head_register_number();
    decrease_stack_size();
    AbstractRegister destination = get_head_register_number();
    list->insert_tail(new SubCommand(CommandParameter::create_register_64(destination),
                                      CommandParameter::create_register_64(source)));

    set_head_type(false);
}

void RegStack::mul() {
    AbstractRegister source = get_head_register_number();
    decrease_stack_size();
    AbstractRegister destination = get_head_register_number();
    list->insert_tail(new IMulCommand(destination, source));

    set_head_type(false);
}

void RegStack::div() {
    AbstractRegister source = get_head_register_number();
    decrease_stack_size();
    AbstractRegister destination = get_head_register_number();

    AbstractRegister rdx_handle =
        list->parent_buffer->descriptors.next_constrained_register(rdx, list);
    AbstractRegister rax_handle =
        list->parent_buffer->descriptors.next_constrained_register(rax, list);

    list->insert_tail(new XorCommand(CommandParameter::create_register_64(rdx_handle),
                                      CommandParameter::create_register_64(rdx_handle)));
    list->insert_tail(new MovCommand(CommandParameter::create_register_64(rax_handle),
                                      CommandParameter::create_register_64(destination)));
    list->insert_tail(new IDivCommand(source, rax_handle, rdx_handle));
    list->insert_tail(new MovCommand(CommandParameter::create_register_64(destination),
                                      CommandParameter::create_register_64(rax_handle)));

    set_head_type(false);
}

RegStackEntry RegStack::get_head_register() {
    return stack[stack.size() - 1];
}

void RegStack::set_head_type(bool is_logical) {
    stack[stack.size() - 1].is_logical = is_logical;
}

bool RegStack::push_next_register(bool is_logical) {
    AbstractRegister reg = list->parent_buffer->descriptors.next_register(list);

    stack.push_back({reg, is_logical});
    return true;
}

void RegStack::decrease_stack_size() {
    stack.pop_back();
}

void RegStack::write_head(AbstractRegister reg) {
    list->insert_tail(
        new MovCommand(CommandParameter::create_register_64(reg),
                        CommandParameter::create_register_64(get_head_register().reg)));
}

void RegStack::pop(AbstractRegister reg) {
    write_head(reg);
    decrease_stack_size();
}

void RegStack::convert_logical_to_number(AbstractRegister reg) {
    list->insert_tail(new MovZXCommand(CommandParameter::create_register_64(reg),
                                        CommandParameter::create_register_8(reg)));
}

void RegStack::convert_number_to_logical(AbstractRegister reg) {
    list->insert_tail(new TestCommand(CommandParameter::create_register_64(reg),
                                       CommandParameter::create_register_64(reg)));
    list->insert_tail(new CSetCommand(reg, COMMAND_SETNE));
}

AbstractRegister RegStack::get_head_register_number() {
    auto head = get_head_register();
    if (head.is_logical)
        convert_logical_to_number(head.reg);
    return head.reg;
}

AbstractRegister RegStack::get_head_register_logical() {
    auto head = get_head_register();
    if (!head.is_logical)
        convert_number_to_logical(head.reg);
    return head.reg;
}

} // namespace bonk::x86_backend