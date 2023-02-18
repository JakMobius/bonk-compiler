
#include "bonk/backend/x86/instructions/x86_mov.hpp"
#include "bonk/backend/x86/x86_command_buffer.hpp"
#include "../utils/test_buffer_eq.hpp"
#include "gtest/gtest.h"

using namespace bonk::x86_backend;

TEST(EncodingX86, MovCommandRegImm) {
    auto* cbuffer = new command_buffer();

    for (int i = 0; i < 16; i++) {
        cbuffer->root_list->insert_tail(
            new mov_command(command_parameter::create_register_64(i),
                            command_parameter::create_imm32(0xBEBEBEBEFAFAFAFAll)));
    }

    auto encoder = cbuffer->to_bytes();
    auto& buffer = encoder->buffer;

    // clang-format off
    const char* correct_buffer =
        "48b8fafafafabebebebe" // mov rax, 0xbebebebefafafafa
        "48b9fafafafabebebebe" // mov rcx, 0xbebebebefafafafa
        "48bafafafafabebebebe" // mov rdx, 0xbebebebefafafafa
        "48bbfafafafabebebebe" // mov rbx, 0xbebebebefafafafa
        "48bcfafafafabebebebe" // mov rsp, 0xbebebebefafafafa
        "48bdfafafafabebebebe" // mov rbp, 0xbebebebefafafafa
        "48befafafafabebebebe" // mov rsi, 0xbebebebefafafafa
        "48bffafafafabebebebe" // mov rdi, 0xbebebebefafafafa
        "49b8fafafafabebebebe" // mov r8,  0xbebebebefafafafa
        "49b9fafafafabebebebe" // mov r9,  0xbebebebefafafafa
        "49bafafafafabebebebe" // mov r10, 0xbebebebefafafafa
        "49bbfafafafabebebebe" // mov r11, 0xbebebebefafafafa
        "49bcfafafafabebebebe" // mov r12, 0xbebebebefafafafa
        "49bdfafafafabebebebe" // mov r13, 0xbebebebefafafafa
        "49befafafafabebebebe" // mov r14, 0xbebebebefafafafa
        "49bffafafafabebebebe";// mov r15, 0xbebebebefafafafa


    test_buffer_eq(correct_buffer, buffer);
}
