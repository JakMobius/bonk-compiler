
#include "bonk/backend/x86/instructions/x86_mov.hpp"
#include "bonk/backend/x86/x86_command_buffer.hpp"
#include "../utils/test_buffer_eq.hpp"
#include "gtest/gtest.h"

using namespace bonk::x86_backend;

TEST(EncodingX86, CSetCommand) {

    auto* cbuffer = new CommandBuffer();

    cbuffer->root_list->insert_tail(new CSetCommand(rax, COMMAND_SETE));
    cbuffer->root_list->insert_tail(new CSetCommand(rax, COMMAND_SETNE));
    cbuffer->root_list->insert_tail(new CSetCommand(rax, COMMAND_SETG));
    cbuffer->root_list->insert_tail(new CSetCommand(rax, COMMAND_SETNG));
    cbuffer->root_list->insert_tail(new CSetCommand(rax, COMMAND_SETL));
    cbuffer->root_list->insert_tail(new CSetCommand(rax, COMMAND_SETNL));

    auto encoder = cbuffer->to_bytes();
    auto& buffer = encoder->buffer;

    // clang-format off

    const char* correct_buffer =
        "0f94c0" // sete   al
        "0f95c0" // setne  al
        "0f9fc0" // setg   al
        "0f9ec0" // setle  al
        "0f9cc0" // setl   al
        "0f9dc0"; // setge  al

    // clang-format on

    test_buffer_eq(correct_buffer, buffer);
}
