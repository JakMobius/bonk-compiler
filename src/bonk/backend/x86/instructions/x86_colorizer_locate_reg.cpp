
#include "x86_colorizer_locate_reg.hpp"

namespace bonk::x86_backend {

LocateRegCommand::LocateRegCommand(AbstractRegister reg, CommandParameter position) {
    if (position.type == PARAMETER_TYPE_IMM32) {
        type = COMMAND_COLORIZER_LOCATE_REG_STACK;
    } else if (position.type == PARAMETER_TYPE_REG_64) {
        type = COMMAND_COLORIZER_LOCATE_REG_REG;
    } else {
        assert(!"Invalid parameter type for locate_reg_command");
    }

    set_read_register(reg);
    set_write_register(reg);

    parameters.resize(2);
    parameters[0] = CommandParameter::create_register_64(reg);
    parameters[1] = position;
}

AbstractRegister LocateRegCommand::get_register_location() {
    assert(type == COMMAND_COLORIZER_LOCATE_REG_REG);
    return parameters[1].reg;
}

// e_machine_register locate_reg_command::get_reg_location() {
//     assert(type == COMMAND_COLORIZER_LOCATE_REG_REG);
//     return
//     list->parent_buffer->descriptors.get_descriptor(parameters[1].reg)->last_register_location;
// }

int LocateRegCommand::get_stack_location() {
    assert(type == COMMAND_COLORIZER_LOCATE_REG_STACK);
    return (int)parameters[1].imm;
}

} // namespace bonk::x86_backend