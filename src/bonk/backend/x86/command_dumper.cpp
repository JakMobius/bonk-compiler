
#include "command_dumper.hpp"
#include "instructions/x86_colorizer_scope.hpp"
#include "instructions/x86_jmp_label.hpp"

namespace bonk::x86_backend {

void command_dumper::depth_padding(FILE* file, int depth, bool is_label) {
    if (!is_label)
        fprintf(file, "   ");
    for (int i = 0; i < depth; i++) {
        fprintf(file, "  ");
    }
}

void command_dumper::dump_register(AsmCommand* command, FILE* file, AbstractRegister reg) {

    if (command_list) {
        AbstractRegisterDescriptor* descriptor =
            command_list->parent_buffer->descriptors.get_descriptor(reg);
        if (descriptor->has_register_constraint) {
            fprintf(file, "%s", ASM_REGISTERS_64[descriptor->last_register_location]);
            return;
        }
    }

    fprintf(file, "_r%llu", reg);
}

void command_dumper::dump_param_register(AsmCommand* command, FILE* file, CommandParameter param) {

    if (command_list) {
        AbstractRegisterDescriptor* descriptor =
            command_list->parent_buffer->descriptors.get_descriptor(param.reg);
        if (descriptor->has_register_constraint) {
            if (param.type == PARAMETER_TYPE_REG_64) {
                fprintf(file, "%s", ASM_REGISTERS_64[descriptor->last_register_location]);
                return;
            } else {
                fprintf(file, "%s", ASM_REGISTERS_8[descriptor->last_register_location]);
                return;
            }
        }
    }

    fprintf(file, "_r%llu", param.reg);
}

void command_dumper::dump_scope_command(AsmCommand* command, FILE* file, int depth) {
    depth_padding(file, depth, false);
    fprintf(file, "colorizer::scope {\n");

    if (command->read_registers.size()) {
        depth_padding(file, depth + 1, false);
        fprintf(file, "; %d read registers (", command->read_registers.size());

        for (int i = 0, j = 0; i < command->read_registers.size(); i++) {
            if (j > 0)
                fprintf(file, ", ");
            dump_register(command, file, command->read_registers[i]);
            j++;
        }
        fprintf(file, ")\n");
    }
    if (command->write_registers.size()) {
        depth_padding(file, depth + 1, false);
        fprintf(file, "; %d write registers (", command->write_registers.size());
        for (int i = 0, j = 0; i < command->write_registers.size(); i++) {
            if (j > 0)
                fprintf(file, ", ");
            dump_register(command, file, command->write_registers[i]);
            j++;
        }
        fprintf(file, ")\n");
    }

    dump_list(((ScopeCommand*)command)->commands, file, depth + 1);
    depth_padding(file, depth, false);
    printf("}");
}

void command_dumper::dump(AsmCommand* command, FILE* file, int depth) {
    if (command->type == COMMAND_COLORIZER_SCOPE) {
        dump_scope_command(command, file, depth);
        return;
    }
    if (command->type == COMMAND_JMP_LABEL) {
        depth_padding(file, depth, true);
        fprintf(file, "L%lu:", ((JmpLabel*)command)->get_index());
        return;
    }
    depth_padding(file, depth, false);
    fprintf(file, "%s", ASM_MNEMONICS[command->type]);
    for (int i = 0; i < command->parameters.size(); i++) {
        if (i != 0)
            fputc(',', file);
        CommandParameter parameter = command->parameters[i];
        if (parameter.type == PARAMETER_TYPE_LABEL) {
            if (parameter.label) {
                fprintf(file, " L%lu", parameter.label->get_index());
            } else {
                fprintf(file, " LNULL");
            }
        } else if (parameter.type == PARAMETER_TYPE_IMM32) {
            fprintf(file, " %lld", parameter.imm);
        } else if (parameter.type == PARAMETER_TYPE_REG_64) {
            fputc(' ', file);
            dump_param_register(command, file, parameter);
        } else if (parameter.type == PARAMETER_TYPE_REG_8) {
            fputc(' ', file);
            dump_param_register(command, file, parameter);
        } else if (parameter.type == PARAMETER_TYPE_MEMORY) {
            CommandParameterMemory mem = parameter.memory;

            fprintf(file, " [");

            bool pad = false;

            if (mem.reg_a != AbstractRegister(-1)) {
                fprintf(file, "%s", ASM_REGISTERS_64[mem.reg_a]);
                if (mem.reg_a_constant != 1)
                    fprintf(file, "*%d", mem.reg_a_constant);
                pad = true;
            }

            if (mem.reg_b != AbstractRegister(-1)) {
                if (pad)
                    fputc(' ', file);
                fprintf(file, "%s", ASM_REGISTERS_64[mem.reg_b]);
                pad = true;
            }

            if (mem.displacement != 0) {
                if (pad)
                    fputc(' ', file);
                if (mem.displacement > 0) {
                    fprintf(file, "+ %d", mem.displacement);
                } else {
                    fprintf(file, "- %d", -mem.displacement);
                }
            }

            fprintf(file, "]");
        } else if (parameter.type == PARAMETER_TYPE_SYMBOL) {
            if (command_list) {
                fprintf(file, " %s",
                        command_list->parent_buffer->file->get_symbol_name(parameter.symbol.symbol)
                            .data());
            } else {
                fprintf(file, "<symbol %d>", parameter.symbol.symbol);
            }
        }
    }
}

void command_dumper::dump_list(struct CommandList* list, FILE* file, int depth) {
    auto old_list = command_list;
    command_list = list;
    for (auto i = list->begin(); i != list->end(); list->next_iterator(&i)) {
        dump(list->get(i), file, depth);
        fputc('\n', file);
    }
    command_list = old_list;
}

} // namespace bonk::x86_backend