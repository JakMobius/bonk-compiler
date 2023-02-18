#pragma once

#include "../../../../utils/bin_heap.hpp"
#include "../instructions/x86_command.hpp"
#include "../x86_abstract_register.hpp"
#include "x86_register_usage.hpp"

namespace bonk::x86_backend {

struct register_colorize_context_state;

}

namespace bonk::x86_backend {

struct register_colorize_context_state {

    std::unordered_map<abstract_register, bin_heap<abstract_register_usage>*> usage_heap_array;

    // Relocation table
    std::unordered_map<abstract_register, abstract_register_descriptor> relocation_info;
    command_list* source;

    register_colorize_context_state(command_list* list, register_colorize_context_state* state);

    bool create_usage_heap_array();

    void add_register_usage(abstract_register, abstract_register_usage usage);

    void duplicate_usages();
};

} // namespace bonk::x86_backend
