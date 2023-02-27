#pragma once

#include "colorizer/x86_colorizer.hpp"
#include "x86_command_buffer.hpp"

namespace bonk::x86_backend {

struct FinalOptimizer {
    CommandBuffer* buffer;
    std::unordered_map<JmpLabel*, JmpLabel*> label_map;

    static void optimize(CommandBuffer* buffer);

    FinalOptimizer(CommandBuffer* buffer);

    void optimize_mov_zeroes() const;

    void remove_useless_movs() const;

    void remove_useless_labels() const;

    bool remove_double_jmps() const;

    bool remove_dead_ends() const;

    void apply_label_map();
};

} // namespace bonk::x86_backend
