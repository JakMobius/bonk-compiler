
#include "bonk/backend/x86/instructions/x86_mov.hpp"
#include "bonk/backend/x86/x86_command_buffer.hpp"
#include "../utils/test_buffer_eq.hpp"
#include "gtest/gtest.h"

using namespace bonk::x86_backend;

TEST(EncodingX86, MovCommandRaxMemRegPlusRegScalePlusDispl) {
    auto* cbuffer = new command_buffer();

    // mov rax, [reg * scale + reg + disp32]
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            if (i == rsp)
                continue;
            cbuffer->root_list->insert_tail(new mov_command(
                command_parameter::create_register_64(rax),
                command_parameter::create_memory(
                    command_parameter_memory::create_reg_const_reg_displ(i, 2, j, -512))));
        }
    }

    auto encoder = cbuffer->to_bytes();
    auto& buffer = encoder->buffer;

    // There are special cases if rsp, rbp, r12 and r13 are in the memory operand.
    // They are not encoded in the same way as the other registers.

    // clang-format off
    const char* correct_buffer =
        "488b844000feffff" // mov    rax,QWORD PTR [rax+rax*2-0x200]
        "488b844100feffff" // mov    rax,QWORD PTR [rcx+rax*2-0x200]
        "488b844200feffff" // mov    rax,QWORD PTR [rdx+rax*2-0x200]
        "488b844300feffff" // mov    rax,QWORD PTR [rbx+rax*2-0x200]
        "488b844400feffff" // mov    rax,QWORD PTR [rsp+rax*2-0x200]
        "488b844500feffff" // mov    rax,QWORD PTR [rbp+rax*2-0x200]
        "488b844600feffff" // mov    rax,QWORD PTR [rsi+rax*2-0x200]
        "488b844700feffff" // mov    rax,QWORD PTR [rdi+rax*2-0x200]
        "498b844000feffff" // mov    rax,QWORD PTR [r8+rax*2-0x200]
        "498b844100feffff" // mov    rax,QWORD PTR [r9+rax*2-0x200]
        "498b844200feffff" // mov    rax,QWORD PTR [r10+rax*2-0x200]
        "498b844300feffff" // mov    rax,QWORD PTR [r11+rax*2-0x200]
        "498b844400feffff" // mov    rax,QWORD PTR [r12+rax*2-0x200]
        "498b844500feffff" // mov    rax,QWORD PTR [r13+rax*2-0x200]
        "498b844600feffff" // mov    rax,QWORD PTR [r14+rax*2-0x200]
        "498b844700feffff" // mov    rax,QWORD PTR [r15+rax*2-0x200]
        "488b844800feffff" // mov    rax,QWORD PTR [rax+rcx*2-0x200]
        "488b844900feffff" // mov    rax,QWORD PTR [rcx+rcx*2-0x200]
        "488b844a00feffff" // mov    rax,QWORD PTR [rdx+rcx*2-0x200]
        "488b844b00feffff" // mov    rax,QWORD PTR [rbx+rcx*2-0x200]
        "488b844c00feffff" // mov    rax,QWORD PTR [rsp+rcx*2-0x200]
        "488b844d00feffff" // mov    rax,QWORD PTR [rbp+rcx*2-0x200]
        "488b844e00feffff" // mov    rax,QWORD PTR [rsi+rcx*2-0x200]
        "488b844f00feffff" // mov    rax,QWORD PTR [rdi+rcx*2-0x200]
        "498b844800feffff" // mov    rax,QWORD PTR [r8+rcx*2-0x200]
        "498b844900feffff" // mov    rax,QWORD PTR [r9+rcx*2-0x200]
        "498b844a00feffff" // mov    rax,QWORD PTR [r10+rcx*2-0x200]
        "498b844b00feffff" // mov    rax,QWORD PTR [r11+rcx*2-0x200]
        "498b844c00feffff" // mov    rax,QWORD PTR [r12+rcx*2-0x200]
        "498b844d00feffff" // mov    rax,QWORD PTR [r13+rcx*2-0x200]
        "498b844e00feffff" // mov    rax,QWORD PTR [r14+rcx*2-0x200]
        "498b844f00feffff" // mov    rax,QWORD PTR [r15+rcx*2-0x200]
        "488b845000feffff" // mov    rax,QWORD PTR [rax+rdx*2-0x200]
        "488b845100feffff" // mov    rax,QWORD PTR [rcx+rdx*2-0x200]
        "488b845200feffff" // mov    rax,QWORD PTR [rdx+rdx*2-0x200]
        "488b845300feffff" // mov    rax,QWORD PTR [rbx+rdx*2-0x200]
        "488b845400feffff" // mov    rax,QWORD PTR [rsp+rdx*2-0x200]
        "488b845500feffff" // mov    rax,QWORD PTR [rbp+rdx*2-0x200]
        "488b845600feffff" // mov    rax,QWORD PTR [rsi+rdx*2-0x200]
        "488b845700feffff" // mov    rax,QWORD PTR [rdi+rdx*2-0x200]
        "498b845000feffff" // mov    rax,QWORD PTR [r8+rdx*2-0x200]
        "498b845100feffff" // mov    rax,QWORD PTR [r9+rdx*2-0x200]
        "498b845200feffff" // mov    rax,QWORD PTR [r10+rdx*2-0x200]
        "498b845300feffff" // mov    rax,QWORD PTR [r11+rdx*2-0x200]
        "498b845400feffff" // mov    rax,QWORD PTR [r12+rdx*2-0x200]
        "498b845500feffff" // mov    rax,QWORD PTR [r13+rdx*2-0x200]
        "498b845600feffff" // mov    rax,QWORD PTR [r14+rdx*2-0x200]
        "498b845700feffff" // mov    rax,QWORD PTR [r15+rdx*2-0x200]
        "488b845800feffff" // mov    rax,QWORD PTR [rax+rbx*2-0x200]
        "488b845900feffff" // mov    rax,QWORD PTR [rcx+rbx*2-0x200]
        "488b845a00feffff" // mov    rax,QWORD PTR [rdx+rbx*2-0x200]
        "488b845b00feffff" // mov    rax,QWORD PTR [rbx+rbx*2-0x200]
        "488b845c00feffff" // mov    rax,QWORD PTR [rsp+rbx*2-0x200]
        "488b845d00feffff" // mov    rax,QWORD PTR [rbp+rbx*2-0x200]
        "488b845e00feffff" // mov    rax,QWORD PTR [rsi+rbx*2-0x200]
        "488b845f00feffff" // mov    rax,QWORD PTR [rdi+rbx*2-0x200]
        "498b845800feffff" // mov    rax,QWORD PTR [r8+rbx*2-0x200]
        "498b845900feffff" // mov    rax,QWORD PTR [r9+rbx*2-0x200]
        "498b845a00feffff" // mov    rax,QWORD PTR [r10+rbx*2-0x200]
        "498b845b00feffff" // mov    rax,QWORD PTR [r11+rbx*2-0x200]
        "498b845c00feffff" // mov    rax,QWORD PTR [r12+rbx*2-0x200]
        "498b845d00feffff" // mov    rax,QWORD PTR [r13+rbx*2-0x200]
        "498b845e00feffff" // mov    rax,QWORD PTR [r14+rbx*2-0x200]
        "498b845f00feffff" // mov    rax,QWORD PTR [r15+rbx*2-0x200]
        "488b846800feffff" // mov    rax,QWORD PTR [rax+rbp*2-0x200]
        "488b846900feffff" // mov    rax,QWORD PTR [rcx+rbp*2-0x200]
        "488b846a00feffff" // mov    rax,QWORD PTR [rdx+rbp*2-0x200]
        "488b846b00feffff" // mov    rax,QWORD PTR [rbx+rbp*2-0x200]
        "488b846c00feffff" // mov    rax,QWORD PTR [rsp+rbp*2-0x200]
        "488b846d00feffff" // mov    rax,QWORD PTR [rbp+rbp*2-0x200]
        "488b846e00feffff" // mov    rax,QWORD PTR [rsi+rbp*2-0x200]
        "488b846f00feffff" // mov    rax,QWORD PTR [rdi+rbp*2-0x200]
        "498b846800feffff" // mov    rax,QWORD PTR [r8+rbp*2-0x200]
        "498b846900feffff" // mov    rax,QWORD PTR [r9+rbp*2-0x200]
        "498b846a00feffff" // mov    rax,QWORD PTR [r10+rbp*2-0x200]
        "498b846b00feffff" // mov    rax,QWORD PTR [r11+rbp*2-0x200]
        "498b846c00feffff" // mov    rax,QWORD PTR [r12+rbp*2-0x200]
        "498b846d00feffff" // mov    rax,QWORD PTR [r13+rbp*2-0x200]
        "498b846e00feffff" // mov    rax,QWORD PTR [r14+rbp*2-0x200]
        "498b846f00feffff" // mov    rax,QWORD PTR [r15+rbp*2-0x200]
        "488b847000feffff" // mov    rax,QWORD PTR [rax+rsi*2-0x200]
        "488b847100feffff" // mov    rax,QWORD PTR [rcx+rsi*2-0x200]
        "488b847200feffff" // mov    rax,QWORD PTR [rdx+rsi*2-0x200]
        "488b847300feffff" // mov    rax,QWORD PTR [rbx+rsi*2-0x200]
        "488b847400feffff" // mov    rax,QWORD PTR [rsp+rsi*2-0x200]
        "488b847500feffff" // mov    rax,QWORD PTR [rbp+rsi*2-0x200]
        "488b847600feffff" // mov    rax,QWORD PTR [rsi+rsi*2-0x200]
        "488b847700feffff" // mov    rax,QWORD PTR [rdi+rsi*2-0x200]
        "498b847000feffff" // mov    rax,QWORD PTR [r8+rsi*2-0x200]
        "498b847100feffff" // mov    rax,QWORD PTR [r9+rsi*2-0x200]
        "498b847200feffff" // mov    rax,QWORD PTR [r10+rsi*2-0x200]
        "498b847300feffff" // mov    rax,QWORD PTR [r11+rsi*2-0x200]
        "498b847400feffff" // mov    rax,QWORD PTR [r12+rsi*2-0x200]
        "498b847500feffff" // mov    rax,QWORD PTR [r13+rsi*2-0x200]
        "498b847600feffff" // mov    rax,QWORD PTR [r14+rsi*2-0x200]
        "498b847700feffff" // mov    rax,QWORD PTR [r15+rsi*2-0x200]
        "488b847800feffff" // mov    rax,QWORD PTR [rax+rdi*2-0x200]
        "488b847900feffff" // mov    rax,QWORD PTR [rcx+rdi*2-0x200]
        "488b847a00feffff" // mov    rax,QWORD PTR [rdx+rdi*2-0x200]
        "488b847b00feffff" // mov    rax,QWORD PTR [rbx+rdi*2-0x200]
        "488b847c00feffff" // mov    rax,QWORD PTR [rsp+rdi*2-0x200]
        "488b847d00feffff" // mov    rax,QWORD PTR [rbp+rdi*2-0x200]
        "488b847e00feffff" // mov    rax,QWORD PTR [rsi+rdi*2-0x200]
        "488b847f00feffff" // mov    rax,QWORD PTR [rdi+rdi*2-0x200]
        "498b847800feffff" // mov    rax,QWORD PTR [r8+rdi*2-0x200]
        "498b847900feffff" // mov    rax,QWORD PTR [r9+rdi*2-0x200]
        "498b847a00feffff" // mov    rax,QWORD PTR [r10+rdi*2-0x200]
        "498b847b00feffff" // mov    rax,QWORD PTR [r11+rdi*2-0x200]
        "498b847c00feffff" // mov    rax,QWORD PTR [r12+rdi*2-0x200]
        "498b847d00feffff" // mov    rax,QWORD PTR [r13+rdi*2-0x200]
        "498b847e00feffff" // mov    rax,QWORD PTR [r14+rdi*2-0x200]
        "498b847f00feffff" // mov    rax,QWORD PTR [r15+rdi*2-0x200]
        "4a8b844000feffff" // mov    rax,QWORD PTR [rax+r8*2-0x200]
        "4a8b844100feffff" // mov    rax,QWORD PTR [rcx+r8*2-0x200]
        "4a8b844200feffff" // mov    rax,QWORD PTR [rdx+r8*2-0x200]
        "4a8b844300feffff" // mov    rax,QWORD PTR [rbx+r8*2-0x200]
        "4a8b844400feffff" // mov    rax,QWORD PTR [rsp+r8*2-0x200]
        "4a8b844500feffff" // mov    rax,QWORD PTR [rbp+r8*2-0x200]
        "4a8b844600feffff" // mov    rax,QWORD PTR [rsi+r8*2-0x200]
        "4a8b844700feffff" // mov    rax,QWORD PTR [rdi+r8*2-0x200]
        "4b8b844000feffff" // mov    rax,QWORD PTR [r8+r8*2-0x200]
        "4b8b844100feffff" // mov    rax,QWORD PTR [r9+r8*2-0x200]
        "4b8b844200feffff" // mov    rax,QWORD PTR [r10+r8*2-0x200]
        "4b8b844300feffff" // mov    rax,QWORD PTR [r11+r8*2-0x200]
        "4b8b844400feffff" // mov    rax,QWORD PTR [r12+r8*2-0x200]
        "4b8b844500feffff" // mov    rax,QWORD PTR [r13+r8*2-0x200]
        "4b8b844600feffff" // mov    rax,QWORD PTR [r14+r8*2-0x200]
        "4b8b844700feffff" // mov    rax,QWORD PTR [r15+r8*2-0x200]
        "4a8b844800feffff" // mov    rax,QWORD PTR [rax+r9*2-0x200]
        "4a8b844900feffff" // mov    rax,QWORD PTR [rcx+r9*2-0x200]
        "4a8b844a00feffff" // mov    rax,QWORD PTR [rdx+r9*2-0x200]
        "4a8b844b00feffff" // mov    rax,QWORD PTR [rbx+r9*2-0x200]
        "4a8b844c00feffff" // mov    rax,QWORD PTR [rsp+r9*2-0x200]
        "4a8b844d00feffff" // mov    rax,QWORD PTR [rbp+r9*2-0x200]
        "4a8b844e00feffff" // mov    rax,QWORD PTR [rsi+r9*2-0x200]
        "4a8b844f00feffff" // mov    rax,QWORD PTR [rdi+r9*2-0x200]
        "4b8b844800feffff" // mov    rax,QWORD PTR [r8+r9*2-0x200]
        "4b8b844900feffff" // mov    rax,QWORD PTR [r9+r9*2-0x200]
        "4b8b844a00feffff" // mov    rax,QWORD PTR [r10+r9*2-0x200]
        "4b8b844b00feffff" // mov    rax,QWORD PTR [r11+r9*2-0x200]
        "4b8b844c00feffff" // mov    rax,QWORD PTR [r12+r9*2-0x200]
        "4b8b844d00feffff" // mov    rax,QWORD PTR [r13+r9*2-0x200]
        "4b8b844e00feffff" // mov    rax,QWORD PTR [r14+r9*2-0x200]
        "4b8b844f00feffff" // mov    rax,QWORD PTR [r15+r9*2-0x200]
        "4a8b845000feffff" // mov    rax,QWORD PTR [rax+r10*2-0x200]
        "4a8b845100feffff" // mov    rax,QWORD PTR [rcx+r10*2-0x200]
        "4a8b845200feffff" // mov    rax,QWORD PTR [rdx+r10*2-0x200]
        "4a8b845300feffff" // mov    rax,QWORD PTR [rbx+r10*2-0x200]
        "4a8b845400feffff" // mov    rax,QWORD PTR [rsp+r10*2-0x200]
        "4a8b845500feffff" // mov    rax,QWORD PTR [rbp+r10*2-0x200]
        "4a8b845600feffff" // mov    rax,QWORD PTR [rsi+r10*2-0x200]
        "4a8b845700feffff" // mov    rax,QWORD PTR [rdi+r10*2-0x200]
        "4b8b845000feffff" // mov    rax,QWORD PTR [r8+r10*2-0x200]
        "4b8b845100feffff" // mov    rax,QWORD PTR [r9+r10*2-0x200]
        "4b8b845200feffff" // mov    rax,QWORD PTR [r10+r10*2-0x200]
        "4b8b845300feffff" // mov    rax,QWORD PTR [r11+r10*2-0x200]
        "4b8b845400feffff" // mov    rax,QWORD PTR [r12+r10*2-0x200]
        "4b8b845500feffff" // mov    rax,QWORD PTR [r13+r10*2-0x200]
        "4b8b845600feffff" // mov    rax,QWORD PTR [r14+r10*2-0x200]
        "4b8b845700feffff" // mov    rax,QWORD PTR [r15+r10*2-0x200]
        "4a8b845800feffff" // mov    rax,QWORD PTR [rax+r11*2-0x200]
        "4a8b845900feffff" // mov    rax,QWORD PTR [rcx+r11*2-0x200]
        "4a8b845a00feffff" // mov    rax,QWORD PTR [rdx+r11*2-0x200]
        "4a8b845b00feffff" // mov    rax,QWORD PTR [rbx+r11*2-0x200]
        "4a8b845c00feffff" // mov    rax,QWORD PTR [rsp+r11*2-0x200]
        "4a8b845d00feffff" // mov    rax,QWORD PTR [rbp+r11*2-0x200]
        "4a8b845e00feffff" // mov    rax,QWORD PTR [rsi+r11*2-0x200]
        "4a8b845f00feffff" // mov    rax,QWORD PTR [rdi+r11*2-0x200]
        "4b8b845800feffff" // mov    rax,QWORD PTR [r8+r11*2-0x200]
        "4b8b845900feffff" // mov    rax,QWORD PTR [r9+r11*2-0x200]
        "4b8b845a00feffff" // mov    rax,QWORD PTR [r10+r11*2-0x200]
        "4b8b845b00feffff" // mov    rax,QWORD PTR [r11+r11*2-0x200]
        "4b8b845c00feffff" // mov    rax,QWORD PTR [r12+r11*2-0x200]
        "4b8b845d00feffff" // mov    rax,QWORD PTR [r13+r11*2-0x200]
        "4b8b845e00feffff" // mov    rax,QWORD PTR [r14+r11*2-0x200]
        "4b8b845f00feffff" // mov    rax,QWORD PTR [r15+r11*2-0x200]
        "4a8b846000feffff" // mov    rax,QWORD PTR [rax+r12*2-0x200]
        "4a8b846100feffff" // mov    rax,QWORD PTR [rcx+r12*2-0x200]
        "4a8b846200feffff" // mov    rax,QWORD PTR [rdx+r12*2-0x200]
        "4a8b846300feffff" // mov    rax,QWORD PTR [rbx+r12*2-0x200]
        "4a8b846400feffff" // mov    rax,QWORD PTR [rsp+r12*2-0x200]
        "4a8b846500feffff" // mov    rax,QWORD PTR [rbp+r12*2-0x200]
        "4a8b846600feffff" // mov    rax,QWORD PTR [rsi+r12*2-0x200]
        "4a8b846700feffff" // mov    rax,QWORD PTR [rdi+r12*2-0x200]
        "4b8b846000feffff" // mov    rax,QWORD PTR [r8+r12*2-0x200]
        "4b8b846100feffff" // mov    rax,QWORD PTR [r9+r12*2-0x200]
        "4b8b846200feffff" // mov    rax,QWORD PTR [r10+r12*2-0x200]
        "4b8b846300feffff" // mov    rax,QWORD PTR [r11+r12*2-0x200]
        "4b8b846400feffff" // mov    rax,QWORD PTR [r12+r12*2-0x200]
        "4b8b846500feffff" // mov    rax,QWORD PTR [r13+r12*2-0x200]
        "4b8b846600feffff" // mov    rax,QWORD PTR [r14+r12*2-0x200]
        "4b8b846700feffff" // mov    rax,QWORD PTR [r15+r12*2-0x200]
        "4a8b846800feffff" // mov    rax,QWORD PTR [rax+r13*2-0x200]
        "4a8b846900feffff" // mov    rax,QWORD PTR [rcx+r13*2-0x200]
        "4a8b846a00feffff" // mov    rax,QWORD PTR [rdx+r13*2-0x200]
        "4a8b846b00feffff" // mov    rax,QWORD PTR [rbx+r13*2-0x200]
        "4a8b846c00feffff" // mov    rax,QWORD PTR [rsp+r13*2-0x200]
        "4a8b846d00feffff" // mov    rax,QWORD PTR [rbp+r13*2-0x200]
        "4a8b846e00feffff" // mov    rax,QWORD PTR [rsi+r13*2-0x200]
        "4a8b846f00feffff" // mov    rax,QWORD PTR [rdi+r13*2-0x200]
        "4b8b846800feffff" // mov    rax,QWORD PTR [r8+r13*2-0x200]
        "4b8b846900feffff" // mov    rax,QWORD PTR [r9+r13*2-0x200]
        "4b8b846a00feffff" // mov    rax,QWORD PTR [r10+r13*2-0x200]
        "4b8b846b00feffff" // mov    rax,QWORD PTR [r11+r13*2-0x200]
        "4b8b846c00feffff" // mov    rax,QWORD PTR [r12+r13*2-0x200]
        "4b8b846d00feffff" // mov    rax,QWORD PTR [r13+r13*2-0x200]
        "4b8b846e00feffff" // mov    rax,QWORD PTR [r14+r13*2-0x200]
        "4b8b846f00feffff" // mov    rax,QWORD PTR [r15+r13*2-0x200]
        "4a8b847000feffff" // mov    rax,QWORD PTR [rax+r14*2-0x200]
        "4a8b847100feffff" // mov    rax,QWORD PTR [rcx+r14*2-0x200]
        "4a8b847200feffff" // mov    rax,QWORD PTR [rdx+r14*2-0x200]
        "4a8b847300feffff" // mov    rax,QWORD PTR [rbx+r14*2-0x200]
        "4a8b847400feffff" // mov    rax,QWORD PTR [rsp+r14*2-0x200]
        "4a8b847500feffff" // mov    rax,QWORD PTR [rbp+r14*2-0x200]
        "4a8b847600feffff" // mov    rax,QWORD PTR [rsi+r14*2-0x200]
        "4a8b847700feffff" // mov    rax,QWORD PTR [rdi+r14*2-0x200]
        "4b8b847000feffff" // mov    rax,QWORD PTR [r8+r14*2-0x200]
        "4b8b847100feffff" // mov    rax,QWORD PTR [r9+r14*2-0x200]
        "4b8b847200feffff" // mov    rax,QWORD PTR [r10+r14*2-0x200]
        "4b8b847300feffff" // mov    rax,QWORD PTR [r11+r14*2-0x200]
        "4b8b847400feffff" // mov    rax,QWORD PTR [r12+r14*2-0x200]
        "4b8b847500feffff" // mov    rax,QWORD PTR [r13+r14*2-0x200]
        "4b8b847600feffff" // mov    rax,QWORD PTR [r14+r14*2-0x200]
        "4b8b847700feffff" // mov    rax,QWORD PTR [r15+r14*2-0x200]
        "4a8b847800feffff" // mov    rax,QWORD PTR [rax+r15*2-0x200]
        "4a8b847900feffff" // mov    rax,QWORD PTR [rcx+r15*2-0x200]
        "4a8b847a00feffff" // mov    rax,QWORD PTR [rdx+r15*2-0x200]
        "4a8b847b00feffff" // mov    rax,QWORD PTR [rbx+r15*2-0x200]
        "4a8b847c00feffff" // mov    rax,QWORD PTR [rsp+r15*2-0x200]
        "4a8b847d00feffff" // mov    rax,QWORD PTR [rbp+r15*2-0x200]
        "4a8b847e00feffff" // mov    rax,QWORD PTR [rsi+r15*2-0x200]
        "4a8b847f00feffff" // mov    rax,QWORD PTR [rdi+r15*2-0x200]
        "4b8b847800feffff" // mov    rax,QWORD PTR [r8+r15*2-0x200]
        "4b8b847900feffff" // mov    rax,QWORD PTR [r9+r15*2-0x200]
        "4b8b847a00feffff" // mov    rax,QWORD PTR [r10+r15*2-0x200]
        "4b8b847b00feffff" // mov    rax,QWORD PTR [r11+r15*2-0x200]
        "4b8b847c00feffff" // mov    rax,QWORD PTR [r12+r15*2-0x200]
        "4b8b847d00feffff" // mov    rax,QWORD PTR [r13+r15*2-0x200]
        "4b8b847e00feffff" // mov    rax,QWORD PTR [r14+r15*2-0x200]
        "4b8b847f00feffff"; // mov    rax,QWORD PTR [r15+r15*2-0x200]
    // clang-format on

    test_buffer_eq(correct_buffer, buffer);
}