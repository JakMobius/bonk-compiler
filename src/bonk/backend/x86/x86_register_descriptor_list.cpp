
#include "x86_register_descriptor_list.hpp"

namespace bonk::x86_backend {

RegisterDescriptorList::RegisterDescriptorList() {
    parent_descriptor_list = nullptr;

    for (int i = 0; i < 16; i++) {
        array.push_back({.last_register_location = MachineRegister(i),
                         .has_register_constraint = true,
                         .located_in_register = false,
                         .located_in_memory = false,
                         .owner = nullptr});
    }
}

RegisterDescriptorList::RegisterDescriptorList(RegisterDescriptorList* other) {
    parent_descriptor_list = other;
    offset = parent_descriptor_list->offset + parent_descriptor_list->array.size();
}

AbstractRegister RegisterDescriptorList::next_register(CommandList* owner) {
    array.push_back({.last_register_location = rinvalid,
                     .has_register_constraint = false,
                     .located_in_register = false,
                     .located_in_memory = false,
                     .owner = owner});
    return array.size() - 1 + offset;
}

AbstractRegister RegisterDescriptorList::next_constrained_register(MachineRegister reg,
                                                                     CommandList* owner) {
    array.push_back({.last_register_location = reg,
                     .has_register_constraint = true,
                     .located_in_register = false,
                     .located_in_memory = false,
                     .owner = owner});
    return array.size() - 1 + offset;
}

AbstractRegisterDescriptor* RegisterDescriptorList::get_descriptor(AbstractRegister i) {
    assert(i >= 0 && i < size());
    if (i < offset)
        return parent_descriptor_list->get_descriptor(i);
    return &array[i - offset];
}

AbstractRegister RegisterDescriptorList::machine_register(MachineRegister reg) {
    // if(parent_descriptor_list) return parent_descriptor_list->machine_register(reg);
    return AbstractRegister(reg);
}

unsigned long RegisterDescriptorList::size() {
    return offset + array.size();
}

} // namespace bonk::x86_backend