
#include "x86_colorizer_reg_preserve.hpp"

namespace bonk::x86_backend {
reg_preserve_command::reg_preserve_command(
    const std::vector<abstract_register>& registers_to_preserve, bool read, bool write) {
    type = COMMAND_COLORIZER_REG_PRESERVE;

    for (int i = 0; i < registers_to_preserve.size(); i++) {
        parameters.push_back(command_parameter::create_register_64(registers_to_preserve[i]));
    }

    if (read) {
        read_registers = registers_to_preserve;
    }

    if (write) {
        write_registers = registers_to_preserve;
    }
}
} // namespace bonk::x86_backend