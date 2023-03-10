#pragma once

namespace bonk::x86_backend {

typedef long long AbstractRegister;
struct AbstractRegisterDescriptor;
struct CommandList;

} // namespace bonk::x86_backend

#include "x86_backend.hpp"
#include "x86_machine_register.hpp"

namespace bonk::x86_backend {

struct AbstractRegisterDescriptor {

    // As symbol-located register may not be stored in a
    // stack frame, it is possible to unite these fields
    // to reduce memory consumption
    union {
        /// Either the current or the last position of this register in the stack frame
        int last_memory_position;
        /// Symbol position of this register
        uint32_t symbol_position;
    };

    /// Either the current or the last machine register where this register was stored
    MachineRegister last_register_location;

    /// Indicates if this register only be located in a machine register
    /// specified in last_register_location field
    bool has_register_constraint;

    /// Indicates if this register has location in data section
    /// specified by symbol_position field
    bool has_symbol_position;

    /// Indicates it this register is located in a real machine register
    /// specified by last_register_location field
    bool located_in_register;

    /// Indicates if this register is stored in the stack frame.
    /// the offset is stored in last_memory_position
    bool located_in_memory;

    /// Indicates if this register is stored in the data section.
    /// The symbol is stored in symbol_position field
    bool located_in_symbol;

    // Registers can only be relocated by their owners
    CommandList* owner;
};

} // namespace bonk::x86_backend
