#pragma once

#include "x86_command.hpp"

namespace bonk::x86_backend {

struct reg_preserve_command : asm_command {
    reg_preserve_command(const std::vector<abstract_register>& registers_to_preserve,
                         bool read = true, bool write = true);

    asm_command* clone() override {
        auto new_command = new reg_preserve_command({}, false, false);
        new_command->parameters = parameters;
        new_command->read_registers = read_registers;
        new_command->write_registers = write_registers;
        return new_command;
    }
};

} // namespace bonk::x86_backend
