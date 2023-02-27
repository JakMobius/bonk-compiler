
#pragma once

#include "x86_abstract_register.hpp"

namespace bonk::x86_backend {

struct RegisterDescriptorList {
    std::vector<AbstractRegisterDescriptor> array {};
    RegisterDescriptorList* parent_descriptor_list = nullptr;
    unsigned long offset = 0;

    explicit RegisterDescriptorList(RegisterDescriptorList* other);

    RegisterDescriptorList();

    unsigned long size();

    AbstractRegister next_constrained_register(MachineRegister reg, CommandList* owner);

    AbstractRegister next_register(CommandList* owner);

    AbstractRegister machine_register(MachineRegister reg);

    AbstractRegisterDescriptor* get_descriptor(AbstractRegister i);
};

} // namespace bonk::x86_backend
