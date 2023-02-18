
#pragma once

#include "x86_abstract_register.hpp"

namespace bonk::x86_backend {

struct register_descriptor_list {
    std::vector<abstract_register_descriptor> array;
    register_descriptor_list* parent_descriptor_list;
    unsigned long offset;

    register_descriptor_list(register_descriptor_list* other);

    register_descriptor_list();

    unsigned long size();

    abstract_register next_constrained_register(e_machine_register reg, command_list* owner);

    abstract_register next_register(command_list* owner);

    abstract_register machine_register(e_machine_register reg);

    abstract_register_descriptor* get_descriptor(abstract_register i);
};

} // namespace bonk::x86_backend
