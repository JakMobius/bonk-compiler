
#include "bonk/backend/x86/instructions/x86_mov.hpp"
#include "bonk/backend/x86/x86_command_buffer.hpp"
#include "../utils/test_buffer_eq.hpp"
#include "gtest/gtest.h"

using namespace bonk::x86_backend;

TEST(EncodingX86, MovCommandRaxMemRegPlusDispl) {
    auto* cbuffer = new command_buffer();

    // mov rax, [reg + disp32]
    for (int i = 0; i < 16; i++) {
        cbuffer->root_list->insert_tail(new mov_command(
            command_parameter::create_register_64(rax),
            command_parameter::create_memory(command_parameter_memory::create_reg_displ(i, -512))));
    }

    auto encoder = cbuffer->to_bytes();
    auto& buffer = encoder->buffer;

    // There are special cases if rsp, rbp, r12 and r13 are in the memory operand.
    // They are not encoded in the same way as the other registers.

    // clang-format off
    const char* correct_buffer =
        "488b8000feffff"   // mov    rax,QWORD PTR [rax-0x200]
        "488b8100feffff"   // mov    rax,QWORD PTR [rcx-0x200]
        "488b8200feffff"   // mov    rax,QWORD PTR [rdx-0x200]
        "488b8300feffff"   // mov    rax,QWORD PTR [rbx-0x200]
        "488b842400feffff" // mov    rax,QWORD PTR [rsp-0x200]
        "488b8500feffff"   // mov    rax,QWORD PTR [rbp-0x200]
        "488b8600feffff"   // mov    rax,QWORD PTR [rsi-0x200]
        "488b8700feffff"   // mov    rax,QWORD PTR [rdi-0x200]
        "498b8000feffff"   // mov    rax,QWORD PTR [r8-0x200]
        "498b8100feffff"   // mov    rax,QWORD PTR [r9-0x200]
        "498b8200feffff"   // mov    rax,QWORD PTR [r10-0x200]
        "498b8300feffff"   // mov    rax,QWORD PTR [r11-0x200]
        "4a8b042500feffff" // mov    rax,QWORD PTR [r12*1-0x200]
        "498b8500feffff"   // mov    rax,QWORD PTR [r13-0x200]
        "498b8600feffff"   // mov    rax,QWORD PTR [r14-0x200]
        "498b8700feffff";   // mov    rax,QWORD PTR [r15-0x200]
    // clang-format on

    test_buffer_eq(correct_buffer, buffer);
}
