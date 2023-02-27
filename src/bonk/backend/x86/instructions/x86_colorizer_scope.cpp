
#include "x86_colorizer_scope.hpp"

namespace bonk::x86_backend {

ScopeCommand::ScopeCommand(CommandList* nested_list) {

    std::set<AbstractRegister> read_registers_tree = {};
    std::set<AbstractRegister> write_registers_tree = {};

    nested_list->append_read_register(&read_registers_tree);
    nested_list->append_write_register(&write_registers_tree);

    type = COMMAND_COLORIZER_SCOPE;

    commands = nested_list;

    for (auto command : commands->commands) {
        for (long long reg : command->read_registers) {
            if (read_registers_tree.erase(reg)) {
                set_read_register(reg);
            }
        }
        for (long long reg : command->write_registers) {
            if (write_registers_tree.erase(reg)) {
                set_write_register(reg);
            }
        }
    }
}

} // namespace bonk::x86_backend