#pragma once

#include "../../../../utils/linear_allocator.hpp"
#include "../x86_abstract_register.hpp"
#include "x86_command.hpp"

namespace bonk::x86_backend {

struct IDivCommand : AsmCommand {
    IDivCommand(AbstractRegister divider, AbstractRegister rax_handle, AbstractRegister rdx_handle);

    void to_bytes(CommandEncoder* buffer) override;

    AsmCommand* clone() override {
        // TODO: this looks hacky
        return new IDivCommand(read_registers[0], read_registers[1], read_registers[2]);
    }
};

} // namespace bonk::x86_backend
