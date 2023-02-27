
#include "x86_final_optimizer.hpp"

namespace bonk::x86_backend {

static bool is_conditional_jump(AsmCommandType type) {
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

void FinalOptimizer::optimize(CommandBuffer* buffer) {
    FinalOptimizer optimizer = {buffer};
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

FinalOptimizer::FinalOptimizer(CommandBuffer* the_buffer) {
    buffer = the_buffer;
}

void FinalOptimizer::apply_label_map() const {
    for (auto i = buffer->root_list->commands.begin(); i != buffer->root_list->commands.end();
         ++i) {
        AsmCommand* command = *i;

        for (auto & parameter : command->parameters) {
            auto param = parameter;
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
                    parameter = param;
            }
        }
    }
    label_map.clear();
}

void FinalOptimizer::remove_useless_movs() const {
    for (auto i = buffer->root_list->commands.begin(); i != buffer->root_list->commands.end();) {
        AsmCommand* command = *i;

        // Removing mov rax, rax commands
        if (command->type == COMMAND_MOV) {
            auto left_param = command->parameters[0];
            auto right_param = command->parameters[1];

            if (left_param.type == PARAMETER_TYPE_REG_64 &&
                right_param.type == PARAMETER_TYPE_REG_64 && left_param.reg == right_param.reg) {

                i = buffer->root_list->commands.erase(i);
                continue;
            }
        }
        ++i;
    }
}

bool FinalOptimizer::remove_double_jmps() const {
    bool result = false;
    for (auto i = buffer->root_list->commands.begin(); i != buffer->root_list->commands.end();) {
        AsmCommand* command = *i;

        // Removing mov rax, rax commands
        if (is_conditional_jump(command->type)) {
            auto* jmp = (JumpCommand*)command;
            auto walker = i;
            ++walker;

            if (walker == buffer->root_list->commands.end())
                break;
            AsmCommand* first_next_command = *walker;
            ++walker;
            if (walker == buffer->root_list->commands.end())
                break;
            AsmCommand* second_next_command = *walker;

            if (jmp->get_label() == second_next_command &&
                first_next_command->type == COMMAND_JMP) {
                ((JumpCommand*)first_next_command)->type = jmp->type;
                ((JumpCommand*)first_next_command)->invert_condition();

                jmp->set_label(nullptr);
                i = buffer->root_list->commands.erase(i);
                result = true;
                continue;
            }
        }
        ++i;
    }
    return result;
}

void FinalOptimizer::remove_useless_labels() const {

    std::vector<bool> label_used;

    for (auto i = buffer->root_list->commands.begin(); i != buffer->root_list->commands.end();) {
        AsmCommand* command = *i;

        for (auto param : command->parameters) {
            if (param.type == PARAMETER_TYPE_LABEL) {
                if (label_used.size() <= param.label->get_index()) {
                    label_used.resize(param.label->get_index() + 1);
                }
                label_used[param.label->get_index()] = true;
            }
        }
        ++i;
    }

    for (auto i = buffer->root_list->commands.begin(); i != buffer->root_list->commands.end();) {
        AsmCommand* command = *i;

        if (command->type == COMMAND_JMP_LABEL) {
            auto* label = (JmpLabel*)command;
            if (!label_used[label->get_index()]) {

                i = buffer->root_list->commands.erase(i);
                continue;
            }
        }
        ++i;
    }
}

bool FinalOptimizer::remove_dead_ends() const {
    bool result = false;
    for (auto i = buffer->root_list->commands.begin(); i != buffer->root_list->commands.end();) {
        AsmCommand* command = *i;
        ++i;

        if (command->type == COMMAND_RET || command->type == COMMAND_JMP) {
            while (i != buffer->root_list->commands.end()) {
                command = *i;
                if (command->type == COMMAND_JMP_LABEL)
                    break;

                i = buffer->root_list->commands.erase(i);
                result = true;
            }
        }
    }
    return result;
}

void FinalOptimizer::optimize_mov_zeroes() const {
    for (auto i = buffer->root_list->commands.begin(); i != buffer->root_list->commands.end(); ++i) {
        AsmCommand* command = *i;

        if (command->type == COMMAND_MOV) {
            auto first_param = command->parameters[0];
            auto second_param = command->parameters[1];
            if (first_param.type == PARAMETER_TYPE_REG_64 &&
                second_param.type == PARAMETER_TYPE_IMM32 && second_param.imm == 0) {
                *i = new XorCommand(CommandParameter::create_register_64(first_param.reg),
                                       CommandParameter::create_register_64(first_param.reg));
            }
        }
    }
}

} // namespace bonk::x86_backend