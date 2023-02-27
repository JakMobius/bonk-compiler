#pragma once

#include <cstdint>

namespace bonk::x86_backend {

enum MachineRegister : unsigned char {
    rax,
    rcx,
    rdx,
    rbx,
    rsp,
    rbp,
    rsi,
    rdi,
    r8,
    r9,
    r10,
    r11,
    r12,
    r13,
    r14,
    r15,
    rinvalid
};

}
