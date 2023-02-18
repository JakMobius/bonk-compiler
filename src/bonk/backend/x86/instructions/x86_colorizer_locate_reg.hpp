#pragma once

#include "../x86_abstract_register.hpp"
#include "../x86_machine_register.hpp"
#include "x86_command.hpp"

namespace bonk::x86_backend {

struct locate_reg_command : asm_command {

    locate_reg_command(abstract_register reg, command_parameter position);

    abstract_register get_register_location();

    int get_stack_location();

    asm_command* clone() override {
        return new locate_reg_command(parameters[0].reg, parameters[1]);
    }
};

} // namespace bonk::x86_backend
