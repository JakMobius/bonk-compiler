
#include "x86_colorizer_scope.hpp"

namespace bonk::x86_backend {

scope_command::scope_command(command_list* nested_list) {

    std::set<abstract_register> read_registers_tree = {};
    std::set<abstract_register> write_registers_tree = {};

    nested_list->append_read_register(&read_registers_tree);
    nested_list->append_write_register(&write_registers_tree);

    type = COMMAND_COLORIZER_SCOPE;

    commands = nested_list;

    for (auto i = commands->begin(); i != commands->end(); commands->next_iterator(&i)) {
        asm_command* command = commands->get(i);
        for (int j = 0; j < command->read_registers.size(); j++) {
            auto reg = command->read_registers[j];
            if (read_registers_tree.erase(reg)) {
                set_read_register(reg);
            }
        }
        for (int j = 0; j < command->write_registers.size(); j++) {
            auto reg = command->write_registers[j];
            if (write_registers_tree.erase(reg)) {
                set_write_register(reg);
            }
        }
    }
}

} // namespace bonk::x86_backend