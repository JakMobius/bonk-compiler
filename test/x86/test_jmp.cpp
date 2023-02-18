
#include "bonk/backend/x86/instructions/x86_mov.hpp"
#include "bonk/backend/x86/x86_command_buffer.hpp"
#include "../utils/test_buffer_eq.hpp"
#include "gtest/gtest.h"

using namespace bonk::x86_backend;

TEST(EncodingX86, JmpCommand) {

    auto* cbuffer = new command_buffer();

    auto* label = new jmp_label(0);
    cbuffer->root_list->insert_tail(label);

    cbuffer->root_list->insert_tail(new jump_command(label, COMMAND_JMP));
    cbuffer->root_list->insert_tail(new jump_command(label, COMMAND_JE));
    cbuffer->root_list->insert_tail(new jump_command(label, COMMAND_JNE));
    cbuffer->root_list->insert_tail(new jump_command(label, COMMAND_JG));
    cbuffer->root_list->insert_tail(new jump_command(label, COMMAND_JNG));
    cbuffer->root_list->insert_tail(new jump_command(label, COMMAND_JL));
    cbuffer->root_list->insert_tail(new jump_command(label, COMMAND_JNL));

    auto encoder = cbuffer->to_bytes();
    encoder->do_emplacements();
    auto& buffer = encoder->buffer;

    // clang-format off

    const char* correct_buffer =
        "e9fbffffff"   // jmp    0x0
        "0f84f5ffffff" // je     0x0
        "0f85efffffff" // jne    0x0
        "0f8fe9ffffff" // jg     0x0
        "0f8ee3ffffff" // jle    0x0
        "0f8cddffffff" // jl     0x0
        "0f8dd7ffffff";// jge    0x0


    // clang-format on

    test_buffer_eq(correct_buffer, buffer);
}
