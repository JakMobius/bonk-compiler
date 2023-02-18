#pragma once

#include "../../../../utils/linear_allocator.hpp"
#include "../x86_abstract_register.hpp"
#include "x86_command.hpp"

namespace bonk::x86_backend {

struct idiv_command : asm_command {
    idiv_command(abstract_register divider, abstract_register rax_handle,
                 abstract_register rdx_handle);

    void to_bytes(command_encoder* buffer) override;

    asm_command* clone() override {
        // TODO: this looks hacky
        return new idiv_command(read_registers[0], read_registers[1], read_registers[2]);
    }
};

} // namespace bonk::x86_backend
