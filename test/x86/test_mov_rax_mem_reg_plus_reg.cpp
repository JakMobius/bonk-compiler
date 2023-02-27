
#include "bonk/backend/x86/instructions/x86_mov.hpp"
#include "bonk/backend/x86/x86_command_buffer.hpp"
#include "../utils/test_buffer_eq.hpp"
#include "gtest/gtest.h"

using namespace bonk::x86_backend;

TEST(EncodingX86, MovCommandRaxMemRegPlusReg) {
    auto* cbuffer = new CommandBuffer();

    // mov rax, [reg + reg]
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            // rsp cannot be used as an index register, so skip it
            if (i == rsp && j == rsp)
                continue;
            cbuffer->root_list->commands.push_back(new MovCommand(
                CommandParameter::create_register_64(0),
                CommandParameter::create_memory(CommandParameterMemory::create_reg_reg(i, j))));
        }
    }

    auto encoder = cbuffer->to_bytes();
    auto& buffer = encoder->buffer;

    // There are special cases if rsp, rbp, r12 and r13 are in the memory operand.
    // They are not encoded in the same way as the other registers.

    // clang-format off
    const char* correct_buffer =
        "488b0400"   // mov    rax,QWORD PTR [rax+rax*1]
        "488b0401"   // mov    rax,QWORD PTR [rcx+rax*1]
        "488b0402"   // mov    rax,QWORD PTR [rdx+rax*1]
        "488b0403"   // mov    rax,QWORD PTR [rbx+rax*1]
        "488b0404"   // mov    rax,QWORD PTR [rsp+rax*1]
        "488b0428"   // mov    rax,QWORD PTR [rax+rbp*1] <- registers are swapped
        "488b0406"   // mov    rax,QWORD PTR [rsi+rax*1]
        "488b0407"   // mov    rax,QWORD PTR [rdi+rax*1]
        "498b0400"   // mov    rax,QWORD PTR [r8+rax*1]
        "498b0401"   // mov    rax,QWORD PTR [r9+rax*1]
        "498b0402"   // mov    rax,QWORD PTR [r10+rax*1]
        "498b0403"   // mov    rax,QWORD PTR [r11+rax*1]
        "498b0404"   // mov    rax,QWORD PTR [r12+rax*1]
        "4a8b0428"   // mov    rax,QWORD PTR [rax+r13*1] <- swap
        "498b0406"   // mov    rax,QWORD PTR [r14+rax*1]
        "498b0407"   // mov    rax,QWORD PTR [r15+rax*1]
        "488b0408"   // mov    rax,QWORD PTR [rax+rcx*1]
        "488b0409"   // mov    rax,QWORD PTR [rcx+rcx*1]
        "488b040a"   // mov    rax,QWORD PTR [rdx+rcx*1]
        "488b040b"   // mov    rax,QWORD PTR [rbx+rcx*1]
        "488b040c"   // mov    rax,QWORD PTR [rsp+rcx*1]
        "488b0429"   // mov    rax,QWORD PTR [rcx+rbp*1] <- swap
        "488b040e"   // mov    rax,QWORD PTR [rsi+rcx*1]
        "488b040f"   // mov    rax,QWORD PTR [rdi+rcx*1]
        "498b0408"   // mov    rax,QWORD PTR [r8+rcx*1]
        "498b0409"   // mov    rax,QWORD PTR [r9+rcx*1]
        "498b040a"   // mov    rax,QWORD PTR [r10+rcx*1]
        "498b040b"   // mov    rax,QWORD PTR [r11+rcx*1]
        "498b040c"   // mov    rax,QWORD PTR [r12+rcx*1]
        "4a8b0429"   // mov    rax,QWORD PTR [rcx+r13*1] <- swap
        "498b040e"   // mov    rax,QWORD PTR [r14+rcx*1]
        "498b040f"   // mov    rax,QWORD PTR [r15+rcx*1]
        "488b0410"   // mov    rax,QWORD PTR [rax+rdx*1]
        "488b0411"   // mov    rax,QWORD PTR [rcx+rdx*1]
        "488b0412"   // mov    rax,QWORD PTR [rdx+rdx*1]
        "488b0413"   // mov    rax,QWORD PTR [rbx+rdx*1]
        "488b0414"   // mov    rax,QWORD PTR [rsp+rdx*1]
        "488b042a"   // mov    rax,QWORD PTR [rdx+rbp*1] <- swap
        "488b0416"   // mov    rax,QWORD PTR [rsi+rdx*1]
        "488b0417"   // mov    rax,QWORD PTR [rdi+rdx*1]
        "498b0410"   // mov    rax,QWORD PTR [r8+rdx*1]
        "498b0411"   // mov    rax,QWORD PTR [r9+rdx*1]
        "498b0412"   // mov    rax,QWORD PTR [r10+rdx*1]
        "498b0413"   // mov    rax,QWORD PTR [r11+rdx*1]
        "498b0414"   // mov    rax,QWORD PTR [r12+rdx*1]
        "4a8b042a"   // mov    rax,QWORD PTR [rdx+r13*1] <- swap
        "498b0416"   // mov    rax,QWORD PTR [r14+rdx*1]
        "498b0417"   // mov    rax,QWORD PTR [r15+rdx*1]
        "488b0418"   // mov    rax,QWORD PTR [rax+rbx*1]
        "488b0419"   // mov    rax,QWORD PTR [rcx+rbx*1]
        "488b041a"   // mov    rax,QWORD PTR [rdx+rbx*1]
        "488b041b"   // mov    rax,QWORD PTR [rbx+rbx*1]
        "488b041c"   // mov    rax,QWORD PTR [rsp+rbx*1]
        "488b042b"   // mov    rax,QWORD PTR [rbx+rbp*1] <- swap
        "488b041e"   // mov    rax,QWORD PTR [rsi+rbx*1]
        "488b041f"   // mov    rax,QWORD PTR [rdi+rbx*1]
        "498b0418"   // mov    rax,QWORD PTR [r8+rbx*1]
        "498b0419"   // mov    rax,QWORD PTR [r9+rbx*1]
        "498b041a"   // mov    rax,QWORD PTR [r10+rbx*1]
        "498b041b"   // mov    rax,QWORD PTR [r11+rbx*1]
        "498b041c"   // mov    rax,QWORD PTR [r12+rbx*1]
        "4a8b042b"   // mov    rax,QWORD PTR [rbx+r13*1] <- swap
        "498b041e"   // mov    rax,QWORD PTR [r14+rbx*1]
        "498b041f"   // mov    rax,QWORD PTR [r15+rbx*1]
        "488b0404"   // mov    rax,QWORD PTR [rsp+rax*1] <- swap (because of rsp)
        "488b040c"   // mov    rax,QWORD PTR [rsp+rcx*1] <- swap (because of rsp)
        "488b0414"   // mov    rax,QWORD PTR [rsp+rdx*1] <- swap (because of rsp)
        "488b041c"   // mov    rax,QWORD PTR [rsp+rbx*1] <- swap (because of rsp)
        /* rsp-rsp case skipped */
        "488b042c"   // mov    rax,QWORD PTR [rsp+rbp*1] <- swap (because of rsp)
        "488b0434"   // mov    rax,QWORD PTR [rsp+rsi*1] <- swap (because of rsp)
        "488b043c"   // mov    rax,QWORD PTR [rsp+rdi*1] <- swap (because of rsp)
        "4a8b0404"   // mov    rax,QWORD PTR [rsp+r8*1] <- swap (because of rsp)
        "4a8b040c"   // mov    rax,QWORD PTR [rsp+r9*1] <- swap (because of rsp)
        "4a8b0414"   // mov    rax,QWORD PTR [rsp+r10*1] <- swap (because of rsp)
        "4a8b041c"   // mov    rax,QWORD PTR [rsp+r11*1] <- swap (because of rsp)
        "4a8b0424"   // mov    rax,QWORD PTR [rsp+r12*1] <- swap (because of rsp)
        "4a8b042c"   // mov    rax,QWORD PTR [rsp+r13*1] <- swap (because of rsp)
        "4a8b0434"   // mov    rax,QWORD PTR [rsp+r14*1] <- swap (because of rsp)
        "4a8b043c"   // mov    rax,QWORD PTR [rsp+r15*1] <- swap (because of rsp)
        "488b0428"   // mov    rax,QWORD PTR [rax+rbp*1]
        "488b0429"   // mov    rax,QWORD PTR [rcx+rbp*1]
        "488b042a"   // mov    rax,QWORD PTR [rdx+rbp*1]
        "488b042b"   // mov    rax,QWORD PTR [rbx+rbp*1]
        "488b042c"   // mov    rax,QWORD PTR [rsp+rbp*1]
        "488b442d00" // mov    rax,QWORD PTR [rbp+rbp*1+0x0] <- displacement added for rbp/rbp case
        "488b042e"   // mov    rax,QWORD PTR [rsi+rbp*1]
        "488b042f"   // mov    rax,QWORD PTR [rdi+rbp*1]
        "498b0428"   // mov    rax,QWORD PTR [r8+rbp*1]
        "498b0429"   // mov    rax,QWORD PTR [r9+rbp*1]
        "498b042a"   // mov    rax,QWORD PTR [r10+rbp*1]
        "498b042b"   // mov    rax,QWORD PTR [r11+rbp*1]
        "498b042c"   // mov    rax,QWORD PTR [r12+rbp*1]
        "498b442d00" // mov    rax,QWORD PTR [r13+rbp*1+0x0] <- displacement added for rbp/r13 case
        "498b042e"   // mov    rax,QWORD PTR [r14+rbp*1]
        "498b042f"   // mov    rax,QWORD PTR [r15+rbp*1]
        "488b0430"   // mov    rax,QWORD PTR [rax+rsi*1]
        "488b0431"   // mov    rax,QWORD PTR [rcx+rsi*1]
        "488b0432"   // mov    rax,QWORD PTR [rdx+rsi*1]
        "488b0433"   // mov    rax,QWORD PTR [rbx+rsi*1]
        "488b0434"   // mov    rax,QWORD PTR [rsp+rsi*1]
        "488b042e"   // mov    rax,QWORD PTR [rsi+rbp*1] <- swap
        "488b0436"   // mov    rax,QWORD PTR [rsi+rsi*1]
        "488b0437"   // mov    rax,QWORD PTR [rdi+rsi*1]
        "498b0430"   // mov    rax,QWORD PTR [r8+rsi*1]
        "498b0431"   // mov    rax,QWORD PTR [r9+rsi*1]
        "498b0432"   // mov    rax,QWORD PTR [r10+rsi*1]
        "498b0433"   // mov    rax,QWORD PTR [r11+rsi*1]
        "498b0434"   // mov    rax,QWORD PTR [r12+rsi*1]
        "4a8b042e"   // mov    rax,QWORD PTR [rsi+r13*1] <- swap
        "498b0436"   // mov    rax,QWORD PTR [r14+rsi*1]
        "498b0437"   // mov    rax,QWORD PTR [r15+rsi*1]
        "488b0438"   // mov    rax,QWORD PTR [rax+rdi*1]
        "488b0439"   // mov    rax,QWORD PTR [rcx+rdi*1]
        "488b043a"   // mov    rax,QWORD PTR [rdx+rdi*1]
        "488b043b"   // mov    rax,QWORD PTR [rbx+rdi*1]
        "488b043c"   // mov    rax,QWORD PTR [rsp+rdi*1]
        "488b042f"   // mov    rax,QWORD PTR [rdi+rbp*1] <- swap
        "488b043e"   // mov    rax,QWORD PTR [rsi+rdi*1]
        "488b043f"   // mov    rax,QWORD PTR [rdi+rdi*1]
        "498b0438"   // mov    rax,QWORD PTR [r8+rdi*1]
        "498b0439"   // mov    rax,QWORD PTR [r9+rdi*1]
        "498b043a"   // mov    rax,QWORD PTR [r10+rdi*1]
        "498b043b"   // mov    rax,QWORD PTR [r11+rdi*1]
        "498b043c"   // mov    rax,QWORD PTR [r12+rdi*1]
        "4a8b042f"   // mov    rax,QWORD PTR [rdi+r13*1] <- swap
        "498b043e"   // mov    rax,QWORD PTR [r14+rdi*1]
        "498b043f"   // mov    rax,QWORD PTR [r15+rdi*1]
        "4a8b0400"   // mov    rax,QWORD PTR [rax+r8*1]
        "4a8b0401"   // mov    rax,QWORD PTR [rcx+r8*1]
        "4a8b0402"   // mov    rax,QWORD PTR [rdx+r8*1]
        "4a8b0403"   // mov    rax,QWORD PTR [rbx+r8*1]
        "4a8b0404"   // mov    rax,QWORD PTR [rsp+r8*1]
        "498b0428"   // mov    rax,QWORD PTR [r8+rbp*1] <- swap
        "4a8b0406"   // mov    rax,QWORD PTR [rsi+r8*1]
        "4a8b0407"   // mov    rax,QWORD PTR [rdi+r8*1]
        "4b8b0400"   // mov    rax,QWORD PTR [r8+r8*1]
        "4b8b0401"   // mov    rax,QWORD PTR [r9+r8*1]
        "4b8b0402"   // mov    rax,QWORD PTR [r10+r8*1]
        "4b8b0403"   // mov    rax,QWORD PTR [r11+r8*1]
        "4b8b0404"   // mov    rax,QWORD PTR [r12+r8*1]
        "4b8b0428"   // mov    rax,QWORD PTR [r8+r13*1] <- swap
        "4b8b0406"   // mov    rax,QWORD PTR [r14+r8*1]
        "4b8b0407"   // mov    rax,QWORD PTR [r15+r8*1]
        "4a8b0408"   // mov    rax,QWORD PTR [rax+r9*1]
        "4a8b0409"   // mov    rax,QWORD PTR [rcx+r9*1]
        "4a8b040a"   // mov    rax,QWORD PTR [rdx+r9*1]
        "4a8b040b"   // mov    rax,QWORD PTR [rbx+r9*1]
        "4a8b040c"   // mov    rax,QWORD PTR [rsp+r9*1]
        "498b0429"   // mov    rax,QWORD PTR [r9+rbp*1] <- swap
        "4a8b040e"   // mov    rax,QWORD PTR [rsi+r9*1]
        "4a8b040f"   // mov    rax,QWORD PTR [rdi+r9*1]
        "4b8b0408"   // mov    rax,QWORD PTR [r8+r9*1]
        "4b8b0409"   // mov    rax,QWORD PTR [r9+r9*1]
        "4b8b040a"   // mov    rax,QWORD PTR [r10+r9*1]
        "4b8b040b"   // mov    rax,QWORD PTR [r11+r9*1]
        "4b8b040c"   // mov    rax,QWORD PTR [r12+r9*1]
        "4b8b0429"   // mov    rax,QWORD PTR [r9+r13*1] <- swap
        "4b8b040e"   // mov    rax,QWORD PTR [r14+r9*1]
        "4b8b040f"   // mov    rax,QWORD PTR [r15+r9*1]
        "4a8b0410"   // mov    rax,QWORD PTR [rax+r10*1]
        "4a8b0411"   // mov    rax,QWORD PTR [rcx+r10*1]
        "4a8b0412"   // mov    rax,QWORD PTR [rdx+r10*1]
        "4a8b0413"   // mov    rax,QWORD PTR [rbx+r10*1]
        "4a8b0414"   // mov    rax,QWORD PTR [rsp+r10*1]
        "498b042a"   // mov    rax,QWORD PTR [r10+rbp*1] <- swap
        "4a8b0416"   // mov    rax,QWORD PTR [rsi+r10*1]
        "4a8b0417"   // mov    rax,QWORD PTR [rdi+r10*1]
        "4b8b0410"   // mov    rax,QWORD PTR [r8+r10*1]
        "4b8b0411"   // mov    rax,QWORD PTR [r9+r10*1]
        "4b8b0412"   // mov    rax,QWORD PTR [r10+r10*1]
        "4b8b0413"   // mov    rax,QWORD PTR [r11+r10*1]
        "4b8b0414"   // mov    rax,QWORD PTR [r12+r10*1]
        "4b8b042a"   // mov    rax,QWORD PTR [r10+r13*1] <- swap
        "4b8b0416"   // mov    rax,QWORD PTR [r14+r10*1]
        "4b8b0417"   // mov    rax,QWORD PTR [r15+r10*1]
        "4a8b0418"   // mov    rax,QWORD PTR [rax+r11*1]
        "4a8b0419"   // mov    rax,QWORD PTR [rcx+r11*1]
        "4a8b041a"   // mov    rax,QWORD PTR [rdx+r11*1]
        "4a8b041b"   // mov    rax,QWORD PTR [rbx+r11*1]
        "4a8b041c"   // mov    rax,QWORD PTR [rsp+r11*1]
        "498b042b"   // mov    rax,QWORD PTR [r11+rbp*1] <- swap
        "4a8b041e"   // mov    rax,QWORD PTR [rsi+r11*1]
        "4a8b041f"   // mov    rax,QWORD PTR [rdi+r11*1]
        "4b8b0418"   // mov    rax,QWORD PTR [r8+r11*1]
        "4b8b0419"   // mov    rax,QWORD PTR [r9+r11*1]
        "4b8b041a"   // mov    rax,QWORD PTR [r10+r11*1]
        "4b8b041b"   // mov    rax,QWORD PTR [r11+r11*1]
        "4b8b041c"   // mov    rax,QWORD PTR [r12+r11*1]
        "4b8b042b"   // mov    rax,QWORD PTR [r11+r13*1] <- swap
        "4b8b041e"   // mov    rax,QWORD PTR [r14+r11*1]
        "4b8b041f"   // mov    rax,QWORD PTR [r15+r11*1]
        "4a8b0420"   // mov    rax,QWORD PTR [rax+r12*1]
        "4a8b0421"   // mov    rax,QWORD PTR [rcx+r12*1]
        "4a8b0422"   // mov    rax,QWORD PTR [rdx+r12*1]
        "4a8b0423"   // mov    rax,QWORD PTR [rbx+r12*1]
        "4a8b0424"   // mov    rax,QWORD PTR [rsp+r12*1]
        "498b042c"   // mov    rax,QWORD PTR [r12+rbp*1] <- swap
        "4a8b0426"   // mov    rax,QWORD PTR [rsi+r12*1]
        "4a8b0427"   // mov    rax,QWORD PTR [rdi+r12*1]
        "4b8b0420"   // mov    rax,QWORD PTR [r8+r12*1]
        "4b8b0421"   // mov    rax,QWORD PTR [r9+r12*1]
        "4b8b0422"   // mov    rax,QWORD PTR [r10+r12*1]
        "4b8b0423"   // mov    rax,QWORD PTR [r11+r12*1]
        "4b8b0424"   // mov    rax,QWORD PTR [r12+r12*1]
        "4b8b042c"   // mov    rax,QWORD PTR [r12+r13*1] <- swap
        "4b8b0426"   // mov    rax,QWORD PTR [r14+r12*1]
        "4b8b0427"   // mov    rax,QWORD PTR [r15+r12*1]
        "4a8b0428"   // mov    rax,QWORD PTR [rax+r13*1]
        "4a8b0429"   // mov    rax,QWORD PTR [rcx+r13*1]
        "4a8b042a"   // mov    rax,QWORD PTR [rdx+r13*1]
        "4a8b042b"   // mov    rax,QWORD PTR [rbx+r13*1]
        "4a8b042c"   // mov    rax,QWORD PTR [rsp+r13*1]
        "4a8b442d00" // mov    rax,QWORD PTR [rbp+r13*1+0x0] <- displacement added for rbp-r13 case
        "4a8b042e"   // mov    rax,QWORD PTR [rsi+r13*1]
        "4a8b042f"   // mov    rax,QWORD PTR [rdi+r13*1]
        "4b8b0428"   // mov    rax,QWORD PTR [r8+r13*1]
        "4b8b0429"   // mov    rax,QWORD PTR [r9+r13*1]
        "4b8b042a"   // mov    rax,QWORD PTR [r10+r13*1]
        "4b8b042b"   // mov    rax,QWORD PTR [r11+r13*1]
        "4b8b042c"   // mov    rax,QWORD PTR [r12+r13*1]
        "4b8b442d00" // mov    rax,QWORD PTR [r13+r13*1+0x0] <- displacement added for r13-r13 case
        "4b8b042e"   // mov    rax,QWORD PTR [r14+r13*1]
        "4b8b042f"   // mov    rax,QWORD PTR [r15+r13*1]
        "4a8b0430"   // mov    rax,QWORD PTR [rax+r14*1]
        "4a8b0431"   // mov    rax,QWORD PTR [rcx+r14*1]
        "4a8b0432"   // mov    rax,QWORD PTR [rdx+r14*1]
        "4a8b0433"   // mov    rax,QWORD PTR [rbx+r14*1]
        "4a8b0434"   // mov    rax,QWORD PTR [rsp+r14*1]
        "498b042e"   // mov    rax,QWORD PTR [r14+rbp*1] <- swap
        "4a8b0436"   // mov    rax,QWORD PTR [rsi+r14*1]
        "4a8b0437"   // mov    rax,QWORD PTR [rdi+r14*1]
        "4b8b0430"   // mov    rax,QWORD PTR [r8+r14*1]
        "4b8b0431"   // mov    rax,QWORD PTR [r9+r14*1]
        "4b8b0432"   // mov    rax,QWORD PTR [r10+r14*1]
        "4b8b0433"   // mov    rax,QWORD PTR [r11+r14*1]
        "4b8b0434"   // mov    rax,QWORD PTR [r12+r14*1]
        "4b8b042e"   // mov    rax,QWORD PTR [r14+r13*1] <- swap
        "4b8b0436"   // mov    rax,QWORD PTR [r14+r14*1]
        "4b8b0437"   // mov    rax,QWORD PTR [r15+r14*1]
        "4a8b0438"   // mov    rax,QWORD PTR [rax+r15*1]
        "4a8b0439"   // mov    rax,QWORD PTR [rcx+r15*1]
        "4a8b043a"   // mov    rax,QWORD PTR [rdx+r15*1]
        "4a8b043b"   // mov    rax,QWORD PTR [rbx+r15*1]
        "4a8b043c"   // mov    rax,QWORD PTR [rsp+r15*1]
        "498b042f"   // mov    rax,QWORD PTR [r15+rbp*1] <- swap
        "4a8b043e"   // mov    rax,QWORD PTR [rsi+r15*1]
        "4a8b043f"   // mov    rax,QWORD PTR [rdi+r15*1]
        "4b8b0438"   // mov    rax,QWORD PTR [r8+r15*1]
        "4b8b0439"   // mov    rax,QWORD PTR [r9+r15*1]
        "4b8b043a"   // mov    rax,QWORD PTR [r10+r15*1]
        "4b8b043b"   // mov    rax,QWORD PTR [r11+r15*1]
        "4b8b043c"   // mov    rax,QWORD PTR [r12+r15*1]
        "4b8b042f"   // mov    rax,QWORD PTR [r15+r13*1] <- swap
        "4b8b043e"   // mov    rax,QWORD PTR [r14+r15*1]
        "4b8b043f";  // mov    rax,QWORD PTR [r15+r15*1]
    // clang-format on

    test_buffer_eq(correct_buffer, buffer);
}