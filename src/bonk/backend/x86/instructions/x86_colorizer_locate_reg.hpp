#pragma once

#include "../x86_abstract_register.hpp"
#include "../x86_machine_register.hpp"
#include "x86_command.hpp"

namespace bonk::x86_backend {

struct LocateRegCommand : AsmCommand {

    LocateRegCommand(AbstractRegister reg, CommandParameter position);

    AbstractRegister get_register_location();

    int get_stack_location();

    AsmCommand* clone() override {
        return new LocateRegCommand(parameters[0].reg, parameters[1]);
    }
};

} // namespace bonk::x86_backend
