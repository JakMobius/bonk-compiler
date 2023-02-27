
#include "x86_conditional_instructions.hpp"

namespace bonk::x86_backend {

void write_jump_placeholder(CommandEncoder* buffer, JmpLabel* label) {
    unsigned long offset_position = buffer->buffer.size();
    for (int i = 0; i < 4; i++)
        buffer->buffer.push_back(0x00);
    unsigned long next_instruction_offset = buffer->buffer.size();

    buffer->request_emplace({.offset = offset_position,
                             .relation = next_instruction_offset,
                             .bytes = 4,
                             .command = label});
}

static void write_jump(CommandEncoder* buffer, char opcode, JmpLabel* label) {
    buffer->buffer.push_back(opcode);
    write_jump_placeholder(buffer, label);
}

static void write_jump(CommandEncoder* buffer, char opcode_a, char opcode_b, JmpLabel* label) {
    buffer->buffer.push_back(opcode_a);
    buffer->buffer.push_back(opcode_b);
    write_jump_placeholder(buffer, label);
}

void JumpCommand::set_label(JmpLabel* label) {
    parameters.resize(1);
    parameters[0] = CommandParameter::create_label(label);
}

JmpLabel* JumpCommand::get_label() {
    return parameters[0].label;
}

void JumpCommand::invert_condition() {
    switch (type) {
    case COMMAND_JE:
        type = COMMAND_JNE;
        break;
    case COMMAND_JNE:
        type = COMMAND_JE;
        break;
    case COMMAND_JL:
        type = COMMAND_JNL;
        break;
    case COMMAND_JNL:
        type = COMMAND_JL;
        break;
    case COMMAND_JG:
        type = COMMAND_JNG;
        break;
    case COMMAND_JNG:
        type = COMMAND_JG;
        break;
    default:
        assert(!"Unconditional jumps may not be inverted");
    }
}

void JumpCommand::to_bytes(CommandEncoder* buffer) {

    CommandParameter target = parameters[0];

    if (target.type == PARAMETER_TYPE_LABEL) {
        switch (type) {
        case COMMAND_JMP:
            write_jump(buffer, 0xE9, target.label);
            break;
        case COMMAND_JE:
            write_jump(buffer, 0x0F, 0x84, target.label);
            break;
        case COMMAND_JNE:
            write_jump(buffer, 0x0F, 0x85, target.label);
            break;
        case COMMAND_JL:
            write_jump(buffer, 0x0F, 0x8C, target.label);
            break;
        case COMMAND_JNL:
            write_jump(buffer, 0x0F, 0x8D, target.label);
            break;
        case COMMAND_JG:
            write_jump(buffer, 0x0F, 0x8F, target.label);
            break;
        case COMMAND_JNG:
            write_jump(buffer, 0x0F, 0x8E, target.label);
            break;
        default:
            assert(!"Cannot encode command");
        }
        return;
    }

    assert(!"Cannot encode command");
}

JumpCommand::JumpCommand(JmpLabel* label, AsmCommandType jump_type) {
    type = jump_type;
    set_label(label);
}

CSetCommand::CSetCommand(AbstractRegister reg, AsmCommandType cset_type) {
    type = cset_type;
    set_read_register(reg);
    set_write_register(reg);

    parameters.resize(1);
    parameters[0] = CommandParameter::create_register_8(reg);
}

void CSetCommand::to_bytes(CommandEncoder* buffer) {
    CommandParameter target = parameters[0];

    if (target.type == PARAMETER_TYPE_REG_8) {
        switch (type) {
        case COMMAND_SETE:
            buffer->write_extended_longopcode(0x0F, 0x94, 0, target);
            break;
        case COMMAND_SETNE:
            buffer->write_extended_longopcode(0x0F, 0x95, 0, target);
            break;
        case COMMAND_SETL:
            buffer->write_extended_longopcode(0x0F, 0x9C, 0, target);
            break;
        case COMMAND_SETNL:
            buffer->write_extended_longopcode(0x0F, 0x9D, 0, target);
            break;
        case COMMAND_SETG:
            buffer->write_extended_longopcode(0x0F, 0x9F, 0, target);
            break;
        case COMMAND_SETNG:
            buffer->write_extended_longopcode(0x0F, 0x9E, 0, target);
            break;
        default:
            assert(!"Cannot encode command");
        }
        return;
    }

    assert(!"Cannot encode command");
}

} // namespace bonk::x86_backend