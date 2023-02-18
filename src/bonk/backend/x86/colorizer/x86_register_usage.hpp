#pragma once

namespace bonk::x86_backend {

struct abstract_register_usage {
    long instruction_index;
    bool is_write;

    bool operator<(const abstract_register_usage& other) const;
};

} // namespace bonk::x86_backend
