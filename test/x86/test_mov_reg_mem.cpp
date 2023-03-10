
#include "bonk/backend/x86/instructions/x86_mov.hpp"
#include "bonk/backend/x86/x86_command_buffer.hpp"
#include "../utils/test_buffer_eq.hpp"
#include "gtest/gtest.h"

using namespace bonk::x86_backend;

TEST(EncodingX86, MovCommandRegRegMem) {
    auto* cbuffer = new CommandBuffer();

    // mov reg, [reg]
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            cbuffer->root_list->commands.push_back(new MovCommand(
                CommandParameter::create_register_64(i),
                CommandParameter::create_memory(CommandParameterMemory::create_reg(j))));
        }
    }

    auto encoder = cbuffer->to_bytes();
    auto& buffer = encoder->buffer;

    // There are special cases if rsp, rbp, r12 and r13 are in the memory operand.
    // They are not encoded in the same way as the other registers.

    // clang-format off
    const char* correct_buffer =
        "488b00"              // mov    rax,QWORD PTR [rax]
        "488b01"              // mov    rax,QWORD PTR [rcx]
        "488b02"              // mov    rax,QWORD PTR [rdx]
        "488b03"              // mov    rax,QWORD PTR [rbx]
        "488b0424"            // mov    rax,QWORD PTR [rsp]
        "488b4500"            // mov    rax,QWORD PTR [rbp+0x0]
        "488b06"              // mov    rax,QWORD PTR [rsi]
        "488b07"              // mov    rax,QWORD PTR [rdi]
        "498b00"              // mov    rax,QWORD PTR [r8]
        "498b01"              // mov    rax,QWORD PTR [r9]
        "498b02"              // mov    rax,QWORD PTR [r10]
        "498b03"              // mov    rax,QWORD PTR [r11]
        "4a8b042500000000"    // mov    rax,QWORD PTR [r12*1+0x0]
        "498b4500"            // mov    rax,QWORD PTR [r13+0x0]
        "498b06"              // mov    rax,QWORD PTR [r14]
        "498b07"              // mov    rax,QWORD PTR [r15]
        "488b08"              // mov    rcx,QWORD PTR [rax]
        "488b09"              // mov    rcx,QWORD PTR [rcx]
        "488b0a"              // mov    rcx,QWORD PTR [rdx]
        "488b0b"              // mov    rcx,QWORD PTR [rbx]
        "488b0c24"            // mov    rcx,QWORD PTR [rsp]
        "488b4d00"            // mov    rcx,QWORD PTR [rbp+0x0]
        "488b0e"              // mov    rcx,QWORD PTR [rsi]
        "488b0f"              // mov    rcx,QWORD PTR [rdi]
        "498b08"              // mov    rcx,QWORD PTR [r8]
        "498b09"              // mov    rcx,QWORD PTR [r9]
        "498b0a"              // mov    rcx,QWORD PTR [r10]
        "498b0b"              // mov    rcx,QWORD PTR [r11]
        "4a8b0c2500000000"    // mov    rcx,QWORD PTR [r12*1+0x0]
        "498b4d00"            // mov    rcx,QWORD PTR [r13+0x0]
        "498b0e"              // mov    rcx,QWORD PTR [r14]
        "498b0f"              // mov    rcx,QWORD PTR [r15]
        "488b10"              // mov    rdx,QWORD PTR [rax]
        "488b11"              // mov    rdx,QWORD PTR [rcx]
        "488b12"              // mov    rdx,QWORD PTR [rdx]
        "488b13"              // mov    rdx,QWORD PTR [rbx]
        "488b1424"            // mov    rdx,QWORD PTR [rsp]
        "488b5500"            // mov    rdx,QWORD PTR [rbp+0x0]
        "488b16"              // mov    rdx,QWORD PTR [rsi]
        "488b17"              // mov    rdx,QWORD PTR [rdi]
        "498b10"              // mov    rdx,QWORD PTR [r8]
        "498b11"              // mov    rdx,QWORD PTR [r9]
        "498b12"              // mov    rdx,QWORD PTR [r10]
        "498b13"              // mov    rdx,QWORD PTR [r11]
        "4a8b142500000000"    // mov    rdx,QWORD PTR [r12*1+0x0]
        "498b5500"            // mov    rdx,QWORD PTR [r13+0x0]
        "498b16"              // mov    rdx,QWORD PTR [r14]
        "498b17"              // mov    rdx,QWORD PTR [r15]
        "488b18"              // mov    rbx,QWORD PTR [rax]
        "488b19"              // mov    rbx,QWORD PTR [rcx]
        "488b1a"              // mov    rbx,QWORD PTR [rdx]
        "488b1b"              // mov    rbx,QWORD PTR [rbx]
        "488b1c24"            // mov    rbx,QWORD PTR [rsp]
        "488b5d00"            // mov    rbx,QWORD PTR [rbp+0x0]
        "488b1e"              // mov    rbx,QWORD PTR [rsi]
        "488b1f"              // mov    rbx,QWORD PTR [rdi]
        "498b18"              // mov    rbx,QWORD PTR [r8]
        "498b19"              // mov    rbx,QWORD PTR [r9]
        "498b1a"              // mov    rbx,QWORD PTR [r10]
        "498b1b"              // mov    rbx,QWORD PTR [r11]
        "4a8b1c2500000000"    // mov    rbx,QWORD PTR [r12*1+0x0]
        "498b5d00"            // mov    rbx,QWORD PTR [r13+0x0]
        "498b1e"              // mov    rbx,QWORD PTR [r14]
        "498b1f"              // mov    rbx,QWORD PTR [r15]
        "488b20"              // mov    rsp,QWORD PTR [rax]
        "488b21"              // mov    rsp,QWORD PTR [rcx]
        "488b22"              // mov    rsp,QWORD PTR [rdx]
        "488b23"              // mov    rsp,QWORD PTR [rbx]
        "488b2424"            // mov    rsp,QWORD PTR [rsp]
        "488b6500"            // mov    rsp,QWORD PTR [rbp+0x0]
        "488b26"              // mov    rsp,QWORD PTR [rsi]
        "488b27"              // mov    rsp,QWORD PTR [rdi]
        "498b20"              // mov    rsp,QWORD PTR [r8]
        "498b21"              // mov    rsp,QWORD PTR [r9]
        "498b22"              // mov    rsp,QWORD PTR [r10]
        "498b23"              // mov    rsp,QWORD PTR [r11]
        "4a8b242500000000"    // mov    rsp,QWORD PTR [r12*1+0x0]
        "498b6500"            // mov    rsp,QWORD PTR [r13+0x0]
        "498b26"              // mov    rsp,QWORD PTR [r14]
        "498b27"              // mov    rsp,QWORD PTR [r15]
        "488b28"              // mov    rbp,QWORD PTR [rax]
        "488b29"              // mov    rbp,QWORD PTR [rcx]
        "488b2a"              // mov    rbp,QWORD PTR [rdx]
        "488b2b"              // mov    rbp,QWORD PTR [rbx]
        "488b2c24"            // mov    rbp,QWORD PTR [rsp]
        "488b6d00"            // mov    rbp,QWORD PTR [rbp+0x0]
        "488b2e"              // mov    rbp,QWORD PTR [rsi]
        "488b2f"              // mov    rbp,QWORD PTR [rdi]
        "498b28"              // mov    rbp,QWORD PTR [r8]
        "498b29"              // mov    rbp,QWORD PTR [r9]
        "498b2a"              // mov    rbp,QWORD PTR [r10]
        "498b2b"              // mov    rbp,QWORD PTR [r11]
        "4a8b2c2500000000"    // mov    rbp,QWORD PTR [r12*1+0x0]
        "498b6d00"            // mov    rbp,QWORD PTR [r13+0x0]
        "498b2e"              // mov    rbp,QWORD PTR [r14]
        "498b2f"              // mov    rbp,QWORD PTR [r15]
        "488b30"              // mov    rsi,QWORD PTR [rax]
        "488b31"              // mov    rsi,QWORD PTR [rcx]
        "488b32"              // mov    rsi,QWORD PTR [rdx]
        "488b33"              // mov    rsi,QWORD PTR [rbx]
        "488b3424"            // mov    rsi,QWORD PTR [rsp]
        "488b7500"            // mov    rsi,QWORD PTR [rbp+0x0]
        "488b36"              // mov    rsi,QWORD PTR [rsi]
        "488b37"              // mov    rsi,QWORD PTR [rdi]
        "498b30"              // mov    rsi,QWORD PTR [r8]
        "498b31"              // mov    rsi,QWORD PTR [r9]
        "498b32"              // mov    rsi,QWORD PTR [r10]
        "498b33"              // mov    rsi,QWORD PTR [r11]
        "4a8b342500000000"    // mov    rsi,QWORD PTR [r12*1+0x0]
        "498b7500"            // mov    rsi,QWORD PTR [r13+0x0]
        "498b36"              // mov    rsi,QWORD PTR [r14]
        "498b37"              // mov    rsi,QWORD PTR [r15]
        "488b38"              // mov    rdi,QWORD PTR [rax]
        "488b39"              // mov    rdi,QWORD PTR [rcx]
        "488b3a"              // mov    rdi,QWORD PTR [rdx]
        "488b3b"              // mov    rdi,QWORD PTR [rbx]
        "488b3c24"            // mov    rdi,QWORD PTR [rsp]
        "488b7d00"            // mov    rdi,QWORD PTR [rbp+0x0]
        "488b3e"              // mov    rdi,QWORD PTR [rsi]
        "488b3f"              // mov    rdi,QWORD PTR [rdi]
        "498b38"              // mov    rdi,QWORD PTR [r8]
        "498b39"              // mov    rdi,QWORD PTR [r9]
        "498b3a"              // mov    rdi,QWORD PTR [r10]
        "498b3b"              // mov    rdi,QWORD PTR [r11]
        "4a8b3c2500000000"    // mov    rdi,QWORD PTR [r12*1+0x0]
        "498b7d00"            // mov    rdi,QWORD PTR [r13+0x0]
        "498b3e"              // mov    rdi,QWORD PTR [r14]
        "498b3f"              // mov    rdi,QWORD PTR [r15]
        "4c8b00"              // mov    r8,QWORD PTR [rax]
        "4c8b01"              // mov    r8,QWORD PTR [rcx]
        "4c8b02"              // mov    r8,QWORD PTR [rdx]
        "4c8b03"              // mov    r8,QWORD PTR [rbx]
        "4c8b0424"            // mov    r8,QWORD PTR [rsp]
        "4c8b4500"            // mov    r8,QWORD PTR [rbp+0x0]
        "4c8b06"              // mov    r8,QWORD PTR [rsi]
        "4c8b07"              // mov    r8,QWORD PTR [rdi]
        "4d8b00"              // mov    r8,QWORD PTR [r8]
        "4d8b01"              // mov    r8,QWORD PTR [r9]
        "4d8b02"              // mov    r8,QWORD PTR [r10]
        "4d8b03"              // mov    r8,QWORD PTR [r11]
        "4e8b042500000000"    // mov    r8,QWORD PTR [r12*1+0x0]
        "4d8b4500"            // mov    r8,QWORD PTR [r13+0x0]
        "4d8b06"              // mov    r8,QWORD PTR [r14]
        "4d8b07"              // mov    r8,QWORD PTR [r15]
        "4c8b08"              // mov    r9,QWORD PTR [rax]
        "4c8b09"              // mov    r9,QWORD PTR [rcx]
        "4c8b0a"              // mov    r9,QWORD PTR [rdx]
        "4c8b0b"              // mov    r9,QWORD PTR [rbx]
        "4c8b0c24"            // mov    r9,QWORD PTR [rsp]
        "4c8b4d00"            // mov    r9,QWORD PTR [rbp+0x0]
        "4c8b0e"              // mov    r9,QWORD PTR [rsi]
        "4c8b0f"              // mov    r9,QWORD PTR [rdi]
        "4d8b08"              // mov    r9,QWORD PTR [r8]
        "4d8b09"              // mov    r9,QWORD PTR [r9]
        "4d8b0a"              // mov    r9,QWORD PTR [r10]
        "4d8b0b"              // mov    r9,QWORD PTR [r11]
        "4e8b0c2500000000"    // mov    r9,QWORD PTR [r12*1+0x0]
        "4d8b4d00"            // mov    r9,QWORD PTR [r13+0x0]
        "4d8b0e"              // mov    r9,QWORD PTR [r14]
        "4d8b0f"              // mov    r9,QWORD PTR [r15]
        "4c8b10"              // mov    r10,QWORD PTR [rax]
        "4c8b11"              // mov    r10,QWORD PTR [rcx]
        "4c8b12"              // mov    r10,QWORD PTR [rdx]
        "4c8b13"              // mov    r10,QWORD PTR [rbx]
        "4c8b1424"            // mov    r10,QWORD PTR [rsp]
        "4c8b5500"            // mov    r10,QWORD PTR [rbp+0x0]
        "4c8b16"              // mov    r10,QWORD PTR [rsi]
        "4c8b17"              // mov    r10,QWORD PTR [rdi]
        "4d8b10"              // mov    r10,QWORD PTR [r8]
        "4d8b11"              // mov    r10,QWORD PTR [r9]
        "4d8b12"              // mov    r10,QWORD PTR [r10]
        "4d8b13"              // mov    r10,QWORD PTR [r11]
        "4e8b142500000000"    // mov    r10,QWORD PTR [r12*1+0x0]
        "4d8b5500"            // mov    r10,QWORD PTR [r13+0x0]
        "4d8b16"              // mov    r10,QWORD PTR [r14]
        "4d8b17"              // mov    r10,QWORD PTR [r15]
        "4c8b18"              // mov    r11,QWORD PTR [rax]
        "4c8b19"              // mov    r11,QWORD PTR [rcx]
        "4c8b1a"              // mov    r11,QWORD PTR [rdx]
        "4c8b1b"              // mov    r11,QWORD PTR [rbx]
        "4c8b1c24"            // mov    r11,QWORD PTR [rsp]
        "4c8b5d00"            // mov    r11,QWORD PTR [rbp+0x0]
        "4c8b1e"              // mov    r11,QWORD PTR [rsi]
        "4c8b1f"              // mov    r11,QWORD PTR [rdi]
        "4d8b18"              // mov    r11,QWORD PTR [r8]
        "4d8b19"              // mov    r11,QWORD PTR [r9]
        "4d8b1a"              // mov    r11,QWORD PTR [r10]
        "4d8b1b"              // mov    r11,QWORD PTR [r11]
        "4e8b1c2500000000"    // mov    r11,QWORD PTR [r12*1+0x0]
        "4d8b5d00"            // mov    r11,QWORD PTR [r13+0x0]
        "4d8b1e"              // mov    r11,QWORD PTR [r14]
        "4d8b1f"              // mov    r11,QWORD PTR [r15]
        "4c8b20"              // mov    r12,QWORD PTR [rax]
        "4c8b21"              // mov    r12,QWORD PTR [rcx]
        "4c8b22"              // mov    r12,QWORD PTR [rdx]
        "4c8b23"              // mov    r12,QWORD PTR [rbx]
        "4c8b2424"            // mov    r12,QWORD PTR [rsp]
        "4c8b6500"            // mov    r12,QWORD PTR [rbp+0x0]
        "4c8b26"              // mov    r12,QWORD PTR [rsi]
        "4c8b27"              // mov    r12,QWORD PTR [rdi]
        "4d8b20"              // mov    r12,QWORD PTR [r8]
        "4d8b21"              // mov    r12,QWORD PTR [r9]
        "4d8b22"              // mov    r12,QWORD PTR [r10]
        "4d8b23"              // mov    r12,QWORD PTR [r11]
        "4e8b242500000000"    // mov    r12,QWORD PTR [r12*1+0x0]
        "4d8b6500"            // mov    r12,QWORD PTR [r13+0x0]
        "4d8b26"              // mov    r12,QWORD PTR [r14]
        "4d8b27"              // mov    r12,QWORD PTR [r15]
        "4c8b28"              // mov    r13,QWORD PTR [rax]
        "4c8b29"              // mov    r13,QWORD PTR [rcx]
        "4c8b2a"              // mov    r13,QWORD PTR [rdx]
        "4c8b2b"              // mov    r13,QWORD PTR [rbx]
        "4c8b2c24"            // mov    r13,QWORD PTR [rsp]
        "4c8b6d00"            // mov    r13,QWORD PTR [rbp+0x0]
        "4c8b2e"              // mov    r13,QWORD PTR [rsi]
        "4c8b2f"              // mov    r13,QWORD PTR [rdi]
        "4d8b28"              // mov    r13,QWORD PTR [r8]
        "4d8b29"              // mov    r13,QWORD PTR [r9]
        "4d8b2a"              // mov    r13,QWORD PTR [r10]
        "4d8b2b"              // mov    r13,QWORD PTR [r11]
        "4e8b2c2500000000"    // mov    r13,QWORD PTR [r12*1+0x0]
        "4d8b6d00"            // mov    r13,QWORD PTR [r13+0x0]
        "4d8b2e"              // mov    r13,QWORD PTR [r14]
        "4d8b2f"              // mov    r13,QWORD PTR [r15]
        "4c8b30"              // mov    r14,QWORD PTR [rax]
        "4c8b31"              // mov    r14,QWORD PTR [rcx]
        "4c8b32"              // mov    r14,QWORD PTR [rdx]
        "4c8b33"              // mov    r14,QWORD PTR [rbx]
        "4c8b3424"            // mov    r14,QWORD PTR [rsp]
        "4c8b7500"            // mov    r14,QWORD PTR [rbp+0x0]
        "4c8b36"              // mov    r14,QWORD PTR [rsi]
        "4c8b37"              // mov    r14,QWORD PTR [rdi]
        "4d8b30"              // mov    r14,QWORD PTR [r8]
        "4d8b31"              // mov    r14,QWORD PTR [r9]
        "4d8b32"              // mov    r14,QWORD PTR [r10]
        "4d8b33"              // mov    r14,QWORD PTR [r11]
        "4e8b342500000000"    // mov    r14,QWORD PTR [r12*1+0x0]
        "4d8b7500"            // mov    r14,QWORD PTR [r13+0x0]
        "4d8b36"              // mov    r14,QWORD PTR [r14]
        "4d8b37"              // mov    r14,QWORD PTR [r15]
        "4c8b38"              // mov    r15,QWORD PTR [rax]
        "4c8b39"              // mov    r15,QWORD PTR [rcx]
        "4c8b3a"              // mov    r15,QWORD PTR [rdx]
        "4c8b3b"              // mov    r15,QWORD PTR [rbx]
        "4c8b3c24"            // mov    r15,QWORD PTR [rsp]
        "4c8b7d00"            // mov    r15,QWORD PTR [rbp+0x0]
        "4c8b3e"              // mov    r15,QWORD PTR [rsi]
        "4c8b3f"              // mov    r15,QWORD PTR [rdi]
        "4d8b38"              // mov    r15,QWORD PTR [r8]
        "4d8b39"              // mov    r15,QWORD PTR [r9]
        "4d8b3a"              // mov    r15,QWORD PTR [r10]
        "4d8b3b"              // mov    r15,QWORD PTR [r11]
        "4e8b3c2500000000"    // mov    r15,QWORD PTR [r12*1+0x0]
        "4d8b7d00"            // mov    r15,QWORD PTR [r13+0x0]
        "4d8b3e"              // mov    r15,QWORD PTR [r14]
        "4d8b3f";             // mov    r15,QWORD PTR [r15]


    test_buffer_eq(correct_buffer, buffer);
}