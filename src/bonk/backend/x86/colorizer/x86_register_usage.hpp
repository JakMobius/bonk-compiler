#pragma once

namespace bonk::x86_backend {

struct AbstractRegisterUsage {
    long instruction_index;
    bool is_write;

    bool operator<(const AbstractRegisterUsage& other) const;
};

} // namespace bonk::x86_backend
