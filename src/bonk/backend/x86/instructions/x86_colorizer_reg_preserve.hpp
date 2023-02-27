#pragma once

#include "x86_command.hpp"

namespace bonk::x86_backend {

struct RegPreserveCommand : AsmCommand {
    RegPreserveCommand(const std::vector<AbstractRegister>& registers_to_preserve,
                         bool read = true, bool write = true);

    AsmCommand* clone() override {
        auto new_command = new RegPreserveCommand({}, false, false);
        new_command->parameters = parameters;
        new_command->read_registers = read_registers;
        new_command->write_registers = write_registers;
        return new_command;
    }
};

} // namespace bonk::x86_backend
