#pragma once

#include "colorizer/x86_colorizer.hpp"
#include "x86_command_buffer.hpp"

namespace bonk::x86_backend {

struct final_optimizer {
    command_buffer* buffer;
    std::unordered_map<jmp_label*, jmp_label*> label_map;

    static void optimize(command_buffer* buffer);

    final_optimizer(command_buffer* buffer);

    void optimize_mov_zeroes();

    void remove_useless_movs();

    void remove_useless_labels();

    bool remove_double_jmps();

    bool remove_dead_ends();

    void apply_label_map();
};

} // namespace bonk::x86_backend
