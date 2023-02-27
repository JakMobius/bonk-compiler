#pragma once

#include "../../../../utils/bin_heap.hpp"
#include "../instructions/x86_command.hpp"
#include "../x86_abstract_register.hpp"
#include "x86_register_usage.hpp"

namespace bonk::x86_backend {

struct RegisterColorizeContextState;

}

namespace bonk::x86_backend {

struct RegisterColorizeContextState {

    std::unordered_map<AbstractRegister, bin_heap<AbstractRegisterUsage>*> usage_heap_array{};

    // Relocation table
    std::unordered_map<AbstractRegister, AbstractRegisterDescriptor> relocation_info{};
    CommandList* source;

    RegisterColorizeContextState(CommandList* list, RegisterColorizeContextState* state);

    bool create_usage_heap_array();

    void add_register_usage(AbstractRegister, AbstractRegisterUsage usage);

    void duplicate_usages();
};

} // namespace bonk::x86_backend
