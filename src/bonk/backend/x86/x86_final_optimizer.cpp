
#include "x86_final_optimizer.hpp"

namespace bonk::x86_backend {

static bool is_conditional_jump(asm_command_type type) {
    switch (type) {
    case COMMAND_JE:
    case COMMAND_JNE:
    case COMMAND_JG:
    case COMMAND_JNG:
    case COMMAND_JL:
    case COMMAND_JNL:
        return true;
    default:
        return false;
    }
}

void final_optimizer::optimize(command_buffer* buffer) {
    final_optimizer optimizer = {buffer};
    optimizer.remove_useless_movs();
    optimizer.apply_label_map();
    optimizer.remove_useless_labels();

    bool could_optimize = true;
    while (could_optimize) {
        could_optimize = false;
        while (optimizer.remove_double_jmps()) {
            optimizer.remove_useless_labels();
            could_optimize = true;
        }

        while (optimizer.remove_dead_ends()) {
            optimizer.remove_useless_labels();
            could_optimize = true;
        }
    }

    optimizer.optimize_mov_zeroes();
}

final_optimizer::final_optimizer(command_buffer* the_buffer) {
    buffer = the_buffer;
}

void final_optimizer::apply_label_map() {
    for (auto i = buffer->root_list->begin(); i != buffer->root_list->end();
         i = buffer->root_list->end()) {
        asm_command* command = buffer->root_list->get(i);

        for (int j = 0; j < command->parameters.size(); j++) {
            auto param = command->parameters[j];
            if (param.type == PARAMETER_TYPE_LABEL) {
                bool relocated = false;

                while (true) {
                    auto it = label_map.find(param.label);
                    if (it == label_map.end())
                        break;
                    relocated = true;
                    param.label = it->second;
                }
                if (relocated)
                    command->parameters[j] = param;
            }
        }
    }
    label_map.clear();
}

void final_optimizer::remove_useless_movs() {
    for (auto i = buffer->root_list->begin(); i != buffer->root_list->end();) {
        asm_command* command = buffer->root_list->get(i);

        // Removing mov rax, rax commands
        if (command->type == COMMAND_MOV) {
            auto left_param = command->parameters[0];
            auto right_param = command->parameters[1];

            if (left_param.type == PARAMETER_TYPE_REG_64 &&
                right_param.type == PARAMETER_TYPE_REG_64 && left_param.reg == right_param.reg) {
                auto old = i;
                i = buffer->root_list->next_iterator(i);
                buffer->root_list->remove(old);
                continue;
            }
        }
        i = buffer->root_list->next_iterator(i);
    }
}

bool final_optimizer::remove_double_jmps() {
    bool result = false;
    for (auto i = buffer->root_list->begin(); i != buffer->root_list->end();) {
        asm_command* command = buffer->root_list->get(i);

        // Removing mov rax, rax commands
        if (is_conditional_jump(command->type)) {
            auto* jmp = (jump_command*)command;
            auto walker = i;
            buffer->root_list->next_iterator(&walker);

            if (walker == buffer->root_list->end())
                break;
            asm_command* first_next_command = buffer->root_list->get(walker);
            buffer->root_list->next_iterator(&walker);
            if (walker == buffer->root_list->end())
                break;
            asm_command* second_next_command = buffer->root_list->get(walker);

            if (jmp->get_label() == second_next_command &&
                first_next_command->type == COMMAND_JMP) {
                ((jump_command*)first_next_command)->type = jmp->type;
                ((jump_command*)first_next_command)->invert_condition();
                auto old = i;
                jmp->set_label(nullptr);
                i = buffer->root_list->next_iterator(i);
                buffer->root_list->remove(old);
                result = true;
                continue;
            }
        }
        i = buffer->root_list->next_iterator(i);
    }
    return result;
}

void final_optimizer::remove_useless_labels() {
    for (auto i = buffer->root_list->begin(); i != buffer->root_list->end();) {
        asm_command* command = buffer->root_list->get(i);

        // Removing labels which are not used in any jumps
        if (command->type == COMMAND_JMP_LABEL) {
            auto label = ((jmp_label*)command);
            if (label->jmps_targeting == 0) {
                auto old = i;
                i = buffer->root_list->next_iterator(i);
                buffer->root_list->remove(old);
                continue;
            }
        }
        i = buffer->root_list->next_iterator(i);
    }
}

bool final_optimizer::remove_dead_ends() {
    bool result = false;
    for (auto i = buffer->root_list->begin(); i != buffer->root_list->end();) {
        asm_command* command = buffer->root_list->get(i);
        i = buffer->root_list->next_iterator(i);

        if (command->type == COMMAND_RET || command->type == COMMAND_JMP) {
            while (i != buffer->root_list->end()) {
                command = buffer->root_list->get(i);
                if (command->type == COMMAND_JMP_LABEL)
                    break;
                auto old = i;
                i = buffer->root_list->next_iterator(i);
                buffer->root_list->remove(old);
                result = true;
            }
        }
    }
    return result;
}

void final_optimizer::optimize_mov_zeroes() {
    for (auto i = buffer->root_list->begin(); i != buffer->root_list->end();
         buffer->root_list->next_iterator(&i)) {
        asm_command* command = buffer->root_list->get(i);

        if (command->type == COMMAND_MOV) {
            auto first_param = command->parameters[0];
            auto second_param = command->parameters[1];
            if (first_param.type == PARAMETER_TYPE_REG_64 &&
                second_param.type == PARAMETER_TYPE_IMM32 && second_param.imm == 0) {
                buffer->root_list->set(
                    i, new xor_command(command_parameter::create_register_64(first_param.reg),
                                       command_parameter::create_register_64(first_param.reg)));
            }
        }
    }
}

} // namespace bonk::x86_backend