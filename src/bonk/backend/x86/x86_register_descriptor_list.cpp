
#include "x86_register_descriptor_list.hpp"

namespace bonk::x86_backend {

register_descriptor_list::register_descriptor_list() {
    parent_descriptor_list = nullptr;

    for (int i = 0; i < 16; i++) {
        array.push_back({.last_register_location = e_machine_register(i),
                         .has_register_constraint = true,
                         .located_in_register = false,
                         .located_in_memory = false,
                         .owner = nullptr});
    }
}

register_descriptor_list::register_descriptor_list(register_descriptor_list* other) {
    parent_descriptor_list = other;
    offset = parent_descriptor_list->offset + parent_descriptor_list->array.size();
}

abstract_register register_descriptor_list::next_register(command_list* owner) {
    array.push_back({.last_register_location = rinvalid,
                     .has_register_constraint = false,
                     .located_in_register = false,
                     .located_in_memory = false,
                     .owner = owner});
    return array.size() - 1 + offset;
}

abstract_register register_descriptor_list::next_constrained_register(e_machine_register reg,
                                                                      command_list* owner) {
    array.push_back({.last_register_location = reg,
                     .has_register_constraint = true,
                     .located_in_register = false,
                     .located_in_memory = false,
                     .owner = owner});
    return array.size() - 1 + offset;
}

abstract_register_descriptor* register_descriptor_list::get_descriptor(abstract_register i) {
    assert(i >= 0 && i < size());
    if (i < offset)
        return parent_descriptor_list->get_descriptor(i);
    return &array[i - offset];
}

abstract_register register_descriptor_list::machine_register(e_machine_register reg) {
    // if(parent_descriptor_list) return parent_descriptor_list->machine_register(reg);
    return abstract_register(reg);
}

unsigned long register_descriptor_list::size() {
    return offset + array.size();
}

} // namespace bonk::x86_backend