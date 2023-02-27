
#include "bonk/backend/x86/instructions/x86_mov.hpp"
#include "bonk/backend/x86/x86_command_buffer.hpp"
#include "../utils/test_buffer_eq.hpp"
#include "gtest/gtest.h"

using namespace bonk::x86_backend;

TEST(EncodingX86, MovCommandRegReg) {
    auto* cbuffer = new CommandBuffer();

    for(int i = 0; i < 16; i++) {
        for(int j = 0; j < 16; j++) {
            cbuffer->root_list->insert_tail(new MovCommand(
                CommandParameter::create_register_64(i), CommandParameter::create_register_64(j)
                    ));
        }
    }

    auto encoder = cbuffer->to_bytes();
    auto& buffer = encoder->buffer;

    // clang-format off
    const char* correct_buffer =
        "4889c0" "4889c8" "4889d0" "4889d8" "4889e0" "4889e8" "4889f0" "4889f8" // mov rax, *
        "4c89c0" "4c89c8" "4c89d0" "4c89d8" "4c89e0" "4c89e8" "4c89f0" "4c89f8"
        "4889c1" "4889c9" "4889d1" "4889d9" "4889e1" "4889e9" "4889f1" "4889f9" // mov rcx, *
        "4c89c1" "4c89c9" "4c89d1" "4c89d9" "4c89e1" "4c89e9" "4c89f1" "4c89f9"
        "4889c2" "4889ca" "4889d2" "4889da" "4889e2" "4889ea" "4889f2" "4889fa" // mov rdx, *
        "4c89c2" "4c89ca" "4c89d2" "4c89da" "4c89e2" "4c89ea" "4c89f2" "4c89fa"
        "4889c3" "4889cb" "4889d3" "4889db" "4889e3" "4889eb" "4889f3" "4889fb" // mov rbx, *
        "4c89c3" "4c89cb" "4c89d3" "4c89db" "4c89e3" "4c89eb" "4c89f3" "4c89fb"
        "4889c4" "4889cc" "4889d4" "4889dc" "4889e4" "4889ec" "4889f4" "4889fc" // mov rsp, *
        "4c89c4" "4c89cc" "4c89d4" "4c89dc" "4c89e4" "4c89ec" "4c89f4" "4c89fc"
        "4889c5" "4889cd" "4889d5" "4889dd" "4889e5" "4889ed" "4889f5" "4889fd" // mov rbp, *
        "4c89c5" "4c89cd" "4c89d5" "4c89dd" "4c89e5" "4c89ed" "4c89f5" "4c89fd"
        "4889c6" "4889ce" "4889d6" "4889de" "4889e6" "4889ee" "4889f6" "4889fe" // mov rsi, *
        "4c89c6" "4c89ce" "4c89d6" "4c89de" "4c89e6" "4c89ee" "4c89f6" "4c89fe"
        "4889c7" "4889cf" "4889d7" "4889df" "4889e7" "4889ef" "4889f7" "4889ff" // mov rdi, *
        "4c89c7" "4c89cf" "4c89d7" "4c89df" "4c89e7" "4c89ef" "4c89f7" "4c89ff"
        "4989c0" "4989c8" "4989d0" "4989d8" "4989e0" "4989e8" "4989f0" "4989f8" // mov r8, *
        "4d89c0" "4d89c8" "4d89d0" "4d89d8" "4d89e0" "4d89e8" "4d89f0" "4d89f8"
        "4989c1" "4989c9" "4989d1" "4989d9" "4989e1" "4989e9" "4989f1" "4989f9" // mov r9, *
        "4d89c1" "4d89c9" "4d89d1" "4d89d9" "4d89e1" "4d89e9" "4d89f1" "4d89f9"
        "4989c2" "4989ca" "4989d2" "4989da" "4989e2" "4989ea" "4989f2" "4989fa" // mov r10, *
        "4d89c2" "4d89ca" "4d89d2" "4d89da" "4d89e2" "4d89ea" "4d89f2" "4d89fa"
        "4989c3" "4989cb" "4989d3" "4989db" "4989e3" "4989eb" "4989f3" "4989fb" // mov r11, *
        "4d89c3" "4d89cb" "4d89d3" "4d89db" "4d89e3" "4d89eb" "4d89f3" "4d89fb"
        "4989c4" "4989cc" "4989d4" "4989dc" "4989e4" "4989ec" "4989f4" "4989fc" // mov r12, *
        "4d89c4" "4d89cc" "4d89d4" "4d89dc" "4d89e4" "4d89ec" "4d89f4" "4d89fc"
        "4989c5" "4989cd" "4989d5" "4989dd" "4989e5" "4989ed" "4989f5" "4989fd" // mov r13, *
        "4d89c5" "4d89cd" "4d89d5" "4d89dd" "4d89e5" "4d89ed" "4d89f5" "4d89fd"
        "4989c6" "4989ce" "4989d6" "4989de" "4989e6" "4989ee" "4989f6" "4989fe" // mov r14, *
        "4d89c6" "4d89ce" "4d89d6" "4d89de" "4d89e6" "4d89ee" "4d89f6" "4d89fe"
        "4989c7" "4989cf" "4989d7" "4989df" "4989e7" "4989ef" "4989f7" "4989ff" // mov r15, *
        "4d89c7" "4d89cf" "4d89d7" "4d89df" "4d89e7" "4d89ef" "4d89f7" "4d89ff";
    // clang-format on

    test_buffer_eq(correct_buffer, buffer);
}