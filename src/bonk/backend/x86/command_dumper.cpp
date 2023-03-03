
#include "command_dumper.hpp"
#include "instructions/x86_colorizer_scope.hpp"
#include "instructions/x86_jmp_label.hpp"

namespace bonk::x86_backend {

void CommandDumper::depth_padding(const OutputStream& file, int depth, bool is_label) {
    if (!is_label)
        file.get_stream() << "   ";
    for (int i = 0; i < depth; i++) {
        file.get_stream() << "  ";
    }
}

void CommandDumper::dump_register(const OutputStream& file, AbstractRegister reg) const {

    if (command_list) {
        AbstractRegisterDescriptor* descriptor =
            command_list->parent_buffer->descriptors.get_descriptor(reg);
        if (descriptor->has_register_constraint) {
            file.get_stream() << ASM_REGISTERS_64[descriptor->last_register_location];
            return;
        }
    }

    file.get_stream() << "_r" << reg;
}

void CommandDumper::dump_param_register(AsmCommand* command, const OutputStream& output, CommandParameter param) const {

    if (command_list) {
        AbstractRegisterDescriptor* descriptor =
            command_list->parent_buffer->descriptors.get_descriptor(param.reg);
        if (descriptor->has_register_constraint) {
            if (param.type == PARAMETER_TYPE_REG_64) {
                output.get_stream() << ASM_REGISTERS_64[descriptor->last_register_location];
            } else {
                output.get_stream() << ASM_REGISTERS_8[descriptor->last_register_location];
            }
            return;
        }
    }

    output.get_stream() << "_r" << param.reg;
}

void CommandDumper::dump_scope_command(AsmCommand* command, const OutputStream& file, int depth) {
    depth_padding(file, depth, false);
    file.get_stream() << "colorizer::scope {\n";

    if (!command->read_registers.empty()) {
        depth_padding(file, depth + 1, false);
        file.get_stream() << "; " << command->read_registers.size() << " read registers (";

        for (int i = 0, j = 0; i < command->read_registers.size(); i++) {
            if (j > 0)
                file.get_stream() << ", ";
            dump_register(file, command->read_registers[i]);
            j++;
        }
        file.get_stream() << ")\n";
    }
    if (!command->write_registers.empty()) {
        depth_padding(file, depth + 1, false);
        file.get_stream() << "; " << command->write_registers.size() << " write registers (";
        for (int i = 0, j = 0; i < command->write_registers.size(); i++) {
            if (j > 0)
                file.get_stream() << ", ";
            dump_register(file, command->write_registers[i]);
            j++;
        }
        file.get_stream() << ")\n";
    }

    dump_list(((ScopeCommand*)command)->commands, file, depth + 1);
    depth_padding(file, depth, false);
    file.get_stream() << "}\n";
}

void CommandDumper::dump(AsmCommand* command, const OutputStream& file, int depth) {
    if (command->type == COMMAND_COLORIZER_SCOPE) {
        dump_scope_command(command, file, depth);
        return;
    }
    if (command->type == COMMAND_JMP_LABEL) {
        depth_padding(file, depth, true);
        file.get_stream() << "L" << ((JmpLabel*)command)->get_index() << ":\n";
        return;
    }
    depth_padding(file, depth, false);
    file.get_stream() << ASM_MNEMONICS[command->type];
    for (int i = 0; i < command->parameters.size(); i++) {
        if (i != 0)
                file.get_stream() << ",";
        CommandParameter parameter = command->parameters[i];
        if (parameter.type == PARAMETER_TYPE_LABEL) {
            if (parameter.label) {
                file.get_stream() << " L" << parameter.label->get_index();
            } else {
                file.get_stream() << " L<unknown>";
            }
        } else if (parameter.type == PARAMETER_TYPE_IMM32) {
            file.get_stream() << " " << parameter.imm;
        } else if (parameter.type == PARAMETER_TYPE_REG_64) {
            file.get_stream() << " ";
            dump_param_register(command, file, parameter);
        } else if (parameter.type == PARAMETER_TYPE_REG_8) {
            file.get_stream() << " ";
            dump_param_register(command, file, parameter);
        } else if (parameter.type == PARAMETER_TYPE_MEMORY) {
            CommandParameterMemory mem = parameter.memory;

            file.get_stream() << " [";

            bool pad = false;

            if (mem.reg_a != AbstractRegister(-1)) {
                file.get_stream() << ASM_REGISTERS_64[mem.reg_a];
                if (mem.reg_a_constant != 1)
                    file.get_stream() << "*" << mem.reg_a_constant;
                pad = true;
            }

            if (mem.reg_b != AbstractRegister(-1)) {
                if (pad)
                    file.get_stream() << " ";
                file.get_stream() << ASM_REGISTERS_64[mem.reg_b];
                pad = true;
            }

            if (mem.displacement != 0) {
                if (pad)
                    file.get_stream() << " ";
                if (mem.displacement > 0) {
                    file.get_stream() << "+ " << mem.displacement;
                } else {
                    file.get_stream() << "- " << -mem.displacement;
                }
            }

            file.get_stream() << "]";
        } else if (parameter.type == PARAMETER_TYPE_SYMBOL) {
            if (command_list) {
                file.get_stream() << " " << command_list->parent_buffer->file->get_symbol_name(parameter.symbol.symbol);
            } else {
                file.get_stream() << " <symbol " << parameter.symbol.symbol << ">";
            }
        }
    }
}

void CommandDumper::dump_list(struct CommandList* list, const OutputStream& output, int depth) {
    auto old_list = command_list;
    command_list = list;
    for (auto & command : list->commands) {
        dump(command, output, depth);
        output.get_stream() << '\n';
    }
    command_list = old_list;
}

} // namespace bonk::x86_backend