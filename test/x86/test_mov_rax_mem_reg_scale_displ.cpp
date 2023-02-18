
#include "bonk/backend/x86/instructions/x86_mov.hpp"
#include "bonk/backend/x86/x86_command_buffer.hpp"
#include "../utils/test_buffer_eq.hpp"
#include "gtest/gtest.h"

using namespace bonk::x86_backend;

TEST(EncodingX86, MovCommandRaxMemRegScalePlusDispl) {
    auto* cbuffer = new command_buffer();

    // mov rax, [reg * 2 + disp32]
    for (int i = 0; i < 16; i++) {
        if (i == rsp)
            continue;

        cbuffer->root_list->insert_tail(
            new mov_command(command_parameter::create_register_64(rax),
                            command_parameter::create_memory(
                                command_parameter_memory::create_reg_const_displ(i, 2, -512))));
    }

    auto encoder = cbuffer->to_bytes();
    auto& buffer = encoder->buffer;

    // There are special cases if rsp, rbp, r12 and r13 are in the memory operand.
    // They are not encoded in the same way as the other registers.

    // clang-format off
    const char* correct_buffer =
        "488b044500feffff" // mov    rax,QWORD PTR [rax*2-0x200]
        "488b044d00feffff" // mov    rax,QWORD PTR [rcx*2-0x200]
        "488b045500feffff" // mov    rax,QWORD PTR [rdx*2-0x200]
        "488b045d00feffff" // mov    rax,QWORD PTR [rbx*2-0x200]
        "488b046d00feffff" // mov    rax,QWORD PTR [rbp*2-0x200]
        "488b047500feffff" // mov    rax,QWORD PTR [rsi*2-0x200]
        "488b047d00feffff" // mov    rax,QWORD PTR [rdi*2-0x200]
        "4a8b044500feffff" // mov    rax,QWORD PTR [r8*2-0x200]
        "4a8b044d00feffff" // mov    rax,QWORD PTR [r9*2-0x200]
        "4a8b045500feffff" // mov    rax,QWORD PTR [r10*2-0x200]
        "4a8b045d00feffff" // mov    rax,QWORD PTR [r11*2-0x200]
        "4a8b046500feffff" // mov    rax,QWORD PTR [r12*2-0x200]
        "4a8b046d00feffff" // mov    rax,QWORD PTR [r13*2-0x200]
        "4a8b047500feffff" // mov    rax,QWORD PTR [r14*2-0x200]
        "4a8b047d00feffff"; // mov    rax,QWORD PTR [r15*2-0x200]
    // clang-format on

    test_buffer_eq(correct_buffer, buffer);
}
