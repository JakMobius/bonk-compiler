
#include "bonk/backend/x86/instructions/x86_mov.hpp"
#include "bonk/backend/x86/x86_command_buffer.hpp"
#include "../utils/test_buffer_eq.hpp"
#include "gtest/gtest.h"

using namespace bonk::x86_backend;

TEST(EncodingX86, MovCommandRegMemReg) {
    auto* cbuffer = new CommandBuffer();

    // mov reg, [reg]
    for(int i = 0; i < 16; i++) {
        for(int j = 0; j < 16; j++) {
            cbuffer->root_list->insert_tail(new MovCommand(
                CommandParameter::create_memory(CommandParameterMemory::create_reg(i)),
                CommandParameter::create_register_64(j)
                    ));
        }
    }

    auto encoder = cbuffer->to_bytes();
    auto& buffer = encoder->buffer;

    // There are special cases if rsp, rbp, r12 and r13 are in the memory operand.
    // They are not encoded in the same way as the other registers.

    // clang-format off
    const char* correct_buffer =
        // mov    QWORD PTR [rax],*
        "488900" "488908" "488910" "488918" "488920" "488928" "488930" "488938"
        "4c8900" "4c8908" "4c8910" "4c8918" "4c8920" "4c8928" "4c8930" "4c8938"
        // mov    QWORD PTR [rcx],*
        "488901" "488909" "488911" "488919" "488921" "488929" "488931" "488939"
        "4c8901" "4c8909" "4c8911" "4c8919" "4c8921" "4c8929" "4c8931" "4c8939"
        // mov    QWORD PTR [rdx],*
        "488902" "48890a" "488912" "48891a" "488922" "48892a" "488932" "48893a"
        "4c8902" "4c890a" "4c8912" "4c891a" "4c8922" "4c892a" "4c8932" "4c893a"
        // mov    QWORD PTR [rbx],*
        "488903" "48890b" "488913" "48891b" "488923" "48892b" "488933" "48893b"
        "4c8903" "4c890b" "4c8913" "4c891b" "4c8923" "4c892b" "4c8933" "4c893b"
        // mov    QWORD PTR [rsp],*
        "48890424" "48890c24" "48891424" "48891c24" "48892424" "48892c24" "48893424" "48893c24"
        "4c890424" "4c890c24" "4c891424" "4c891c24" "4c892424" "4c892c24" "4c893424" "4c893c24"
        // mov    QWORD PTR [rbp+0x0],*
        "48894500" "48894d00" "48895500" "48895d00" "48896500" "48896d00" "48897500" "48897d00"
        "4c894500" "4c894d00" "4c895500" "4c895d00" "4c896500" "4c896d00" "4c897500" "4c897d00"
        // mov    QWORD PTR [rsi],*
        "488906" "48890e" "488916" "48891e" "488926" "48892e" "488936" "48893e"
        "4c8906" "4c890e" "4c8916" "4c891e" "4c8926" "4c892e" "4c8936" "4c893e"
        // mov    QWORD PTR [rdi],*
        "488907" "48890f" "488917" "48891f" "488927" "48892f" "488937" "48893f"
        "4c8907" "4c890f" "4c8917" "4c891f" "4c8927" "4c892f" "4c8937" "4c893f"
        // mov   QWORD PTR [r8],*
        "498900" "498908" "498910" "498918" "498920" "498928" "498930" "498938"
        "4d8900" "4d8908" "4d8910" "4d8918" "4d8920" "4d8928" "4d8930" "4d8938"
        // mov    QWORD PTR [r9],*
        "498901" "498909" "498911" "498919" "498921" "498929" "498931" "498939"
        "4d8901" "4d8909" "4d8911" "4d8919" "4d8921" "4d8929" "4d8931" "4d8939"
        // mov    QWORD PTR [r10],*
        "498902" "49890a" "498912" "49891a" "498922" "49892a" "498932" "49893a"
        "4d8902" "4d890a" "4d8912" "4d891a" "4d8922" "4d892a" "4d8932" "4d893a"
        // mov    QWORD PTR [r11],*
        "498903" "49890b" "498913" "49891b" "498923" "49892b" "498933" "49893b"
        "4d8903" "4d890b" "4d8913" "4d891b" "4d8923" "4d892b" "4d8933" "4d893b"
        // mov    QWORD PTR [r12*1+0x0],*
        "4a89042500000000" "4a890c2500000000" "4a89142500000000" "4a891c2500000000"
        "4a89242500000000" "4a892c2500000000" "4a89342500000000" "4a893c2500000000"
        "4e89042500000000" "4e890c2500000000" "4e89142500000000" "4e891c2500000000"
        "4e89242500000000" "4e892c2500000000" "4e89342500000000" "4e893c2500000000"
        // mov    QWORD PTR [r13+0x0],*
        "49894500" "49894d00" "49895500" "49895d00" "49896500" "49896d00" "49897500" "49897d00"
        "4d894500" "4d894d00" "4d895500" "4d895d00" "4d896500" "4d896d00" "4d897500" "4d897d00"
        // mov    QWORD PTR [r14],*
        "498906" "49890e" "498916" "49891e" "498926" "49892e" "498936" "49893e"
        "4d8906" "4d890e" "4d8916" "4d891e" "4d8926" "4d892e" "4d8936" "4d893e"
        // mov    QWORD PTR [r15],*
        "498907" "49890f" "498917" "49891f" "498927" "49892f" "498937" "49893f"
        "4d8907" "4d890f" "4d8917" "4d891f" "4d8927" "4d892f" "4d8937" "4d893f";
    // clang-format on

    test_buffer_eq(correct_buffer, buffer);
}
