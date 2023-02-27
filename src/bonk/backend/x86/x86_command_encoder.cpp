
#include "x86_command_encoder.hpp"
#include "instructions/x86_jmp_label.hpp"

namespace bonk::x86_backend {

CommandEncoder::CommandEncoder() {
}

void CommandEncoder::request_emplace(EmplaceRequest request) {
    emplace_requests.push_back(request);
}

void CommandEncoder::do_emplacements() {
    for (int i = 0; i < emplace_requests.size(); i++) {
        auto& request = emplace_requests[i];
        if (!request.command)
            continue;
        long value = request.command->offset - request.relation;
        for (int j = 0; j < request.bytes; j++) {
            buffer[j + request.offset] = ((char*)&value)[j];
        }
    }
}

void CommandEncoder::request_relocation(RelocationRequest request) {
    relocation_requests.push_back(request);
}

// To understand following functions, see
// http://www.cs.loyola.edu/~binkley/371/Encoding_Real_x86_Instructions.html
// Chapters 5-11

bool CommandEncoder::is_sip(CommandParameter reg_rm) {
    if (reg_rm.type == PARAMETER_TYPE_MEMORY) {
        int registers = reg_rm.memory.register_amount();
        if (registers > 1 || registers == 0)
            return true;
        if ((reg_rm.memory.reg_a & 0b111) == rsp)
            return true;
        if (reg_rm.memory.reg_a_constant != 1)
            return true;
        return false;
    } else {
        return false;
    }
}

char CommandEncoder::get_displacement_bytes(CommandParameter reg_rm,
                                            RegisterExtensions* extensions) {
    assert(reg_rm.type == PARAMETER_TYPE_REG_64 || reg_rm.type == PARAMETER_TYPE_REG_8 ||
           reg_rm.type == PARAMETER_TYPE_MEMORY || reg_rm.type == PARAMETER_TYPE_IMM32 ||
           reg_rm.type == PARAMETER_TYPE_SYMBOL);

    if (reg_rm.type == PARAMETER_TYPE_REG_64 || reg_rm.type == PARAMETER_TYPE_REG_8) {
        return 0;
    } else if (reg_rm.type == PARAMETER_TYPE_IMM32 || reg_rm.type == PARAMETER_TYPE_SYMBOL) {
        // 32-bit Displacement-only mode
        return 4;
    }

    // PARAMETER_TYPE_MEMORY
    char size = 0;
    if (reg_rm.memory.displacement == 0)
        size = 0;
    else if (reg_rm.memory.displacement >= INT8_MIN && reg_rm.memory.displacement <= INT8_MAX)
        size = 1;
    else
        size = 4;

    if (is_sip(reg_rm)) {
        bool requires_mask = false;
        char base = 0;
        get_sib_base_index(reg_rm, nullptr, &base, &requires_mask);
        if (requires_mask) {
            if (size == 0)
                size = 1;
        } else if ((base & 0b111) == rbp)
            size = 4; // Displacement-only
    } else {
        if (size == 0) {
            char reg = to_machine_register(reg_rm.memory.get_register(0));
            // [rbp] addressing is unavailable.
            // [rbp + disp8] or [rbp + disp32] should be used instead
            if ((reg & 0b111) == rbp)
                size = 1;
        }
    }
    return size;
}

char CommandEncoder::get_mod_reg_rm_byte(CommandParameter reg, CommandParameter reg_rm,
                                         RegisterExtensions* extensions) {

    assert(reg.type == PARAMETER_TYPE_REG_64 || reg.type == PARAMETER_TYPE_REG_8);
    assert(reg_rm.type == PARAMETER_TYPE_REG_64 || reg_rm.type == PARAMETER_TYPE_REG_8 ||
           reg_rm.type == PARAMETER_TYPE_MEMORY || reg_rm.type == PARAMETER_TYPE_IMM32 ||
           reg_rm.type == PARAMETER_TYPE_SYMBOL);

    char mod_mask = 0b00;
    char rm_mask = 0b000;

    if (reg_rm.type == PARAMETER_TYPE_REG_64 || reg_rm.type == PARAMETER_TYPE_REG_8) {
        mod_mask = 0b11;
        rm_mask = to_machine_register(reg_rm.reg);
        if (rm_mask > 7)
            extensions->b = true;
        rm_mask &= 0b111;
    } else if (reg_rm.type == PARAMETER_TYPE_IMM32 || reg_rm.type == PARAMETER_TYPE_SYMBOL) {
        mod_mask = 0b00;
        rm_mask = 0b101;
    } else if (reg_rm.type == PARAMETER_TYPE_MEMORY) {
        if (reg_rm.memory.displacement == 0)
            mod_mask = 0b00;
        else if (reg_rm.memory.displacement >= INT8_MIN && reg_rm.memory.displacement <= INT8_MAX)
            mod_mask = 0b01;
        else
            mod_mask = 0b10;

        if (is_sip(reg_rm)) {
            // SIP mode
            rm_mask = 0b100;

            bool requires_mask = false;
            char base = 0;
            get_sib_base_index(reg_rm, nullptr, &base, &requires_mask);
            if (requires_mask) {
                if (mod_mask == 0b00)
                    mod_mask = 0b01;
            } else if ((base & 0b111) == rbp) {
                mod_mask = 0b00;
            }
        } else {
            rm_mask = to_machine_register(reg_rm.memory.get_register(0));
            // [rbp] addressing is unavailable.
            // [rbp + disp8] should be used instead
            if ((rm_mask & 0b111) == rbp && mod_mask == 0b00)
                mod_mask = 0b01;
        }
    }

    if (reg.reg > 7)
        extensions->r = true;
    if (rm_mask > 7)
        extensions->b = true;

    reg.reg &= 0b111;
    rm_mask &= 0b111;

    return (mod_mask << 6) | (reg.reg << 3) | (rm_mask << 0);
}

void CommandEncoder::get_sib_base_index(CommandParameter reg_rm, char* index_p, char* base_p,
                                         bool* requires_mask) {
    int register_amount = reg_rm.memory.register_amount();

    char index = 0;
    char base = 0;
    bool base_requires_mask = false;

    if (register_amount > 0)
        index = to_machine_register(reg_rm.memory.get_register(0));
    else
        index = 0b100;
    if (register_amount > 1) {
        base = to_machine_register(reg_rm.memory.get_register(1));
        base_requires_mask = (base & 0b111) == rbp;
    } else
        base = 0b101;

    if (base_requires_mask && (base & 0b111) != (index & 0b111) && (base & 0b111) != rsp &&
        reg_rm.memory.reg_a_constant == 1) {
        // Try to turn registers in reverse order if it is possible
        char temp = index;
        index = base;
        base = temp;
        base_requires_mask = false;
    }

    if (register_amount > 0) {
        // rsp cannot be index register.
        if ((index & 0b111) == rsp) {
            if (register_amount == 1) {
                if (index == rsp) {
                    assert(reg_rm.memory.reg_a_constant == 1);
                    base = index;
                    // in case index is extended, we should enforce it to
                    // use invalid rsp mode
                    index = rsp;
                }
            } else if (index == rsp && register_amount == 2) {
                // Try to exchange base and index registers
                // Make sure it's possible
                assert(reg_rm.memory.reg_a_constant == 1 && base != index);
                char temp = index;
                index = base;
                base = temp;
            }
            if (register_amount > 1 && (base & 0b111) == rbp) {
                base_requires_mask = true;
            }
        }
    }

    if (base_p)
        *base_p = base;
    if (index_p)
        *index_p = index;
    if (requires_mask)
        *requires_mask = base_requires_mask;
}

char CommandEncoder::get_sib_byte(CommandParameter reg_rm, RegisterExtensions* extensions) {
    assert(reg_rm.type == PARAMETER_TYPE_MEMORY);

    char scale = 0;
    char index = 0;
    char base = 0;

    get_sib_base_index(reg_rm, &index, &base, nullptr);

    if (index > 7)
        extensions->x = true;
    if (base > 7)
        extensions->b = true;

    index &= 0b111;
    base &= 0b111;

    switch (reg_rm.memory.reg_a_constant) {
    case 1:
        scale = 0;
        break;
    case 2:
        scale = 1;
        break;
    case 4:
        scale = 2;
        break;
    case 8:
        scale = 4;
        break;
    default:
        assert(!"Invalid scale");
    }

    return (scale << 6) | (index << 3) | (base << 0);
}

void CommandEncoder::write_prefix_opcode_modrm_sib(char opcode, CommandParameter reg,
                                                    CommandParameter reg_rm) {
    assert(reg.type == PARAMETER_TYPE_REG_64 || reg.type == PARAMETER_TYPE_REG_8);

    RegisterExtensions extensions = {.w = reg.type == PARAMETER_TYPE_REG_64};

    bool sib = is_sip(reg_rm);
    char mod_reg_rm = get_mod_reg_rm_byte(reg, reg_rm, &extensions);
    char sib_byte = sib ? get_sib_byte(reg_rm, &extensions) : 0;
    char displacement_size = get_displacement_bytes(reg_rm, &extensions);
    uint64_t displacement = get_displacement(reg_rm);

    if (extensions.exist())
        buffer.push_back(extensions.get_byte());
    buffer.push_back(opcode);
    buffer.push_back(mod_reg_rm);
    if (sib)
        buffer.push_back(sib_byte);

    if (reg_rm.type == PARAMETER_TYPE_SYMBOL) {
        request_relocation(RelocationRequest{
            .relocation = reg_rm.symbol.symbol,
            .address = (int32_t)(buffer.size()),
            .type = macho::RELOCATION_TYPE_CONSTANT,
            .data_length = 4,
            .pc_rel = true,
        });
    }

    for (int i = 0; i < displacement_size; i++) {
        buffer.push_back(((char*)&displacement)[i]);
    }
}

void CommandEncoder::write_prefix_longopcode_regrm_sib(char opcode_a, char opcode_b,
                                                        CommandParameter reg,
                                                        CommandParameter reg_rm) {
    assert(reg.type == PARAMETER_TYPE_REG_64 || reg.type == PARAMETER_TYPE_REG_8);
    RegisterExtensions extensions = {.w = reg.type == PARAMETER_TYPE_REG_64};

    bool sib = is_sip(reg_rm);
    char mod_reg_rm = get_mod_reg_rm_byte(reg, reg_rm, &extensions);
    char sib_byte = sib ? get_sib_byte(reg_rm, &extensions) : 0;
    char displacement_size = get_displacement_bytes(reg_rm, &extensions);
    uint64_t displacement = get_displacement(reg_rm);

    if (extensions.exist())
        buffer.push_back(extensions.get_byte());
    buffer.push_back(opcode_a);
    buffer.push_back(opcode_b);
    buffer.push_back(mod_reg_rm);
    if (sib)
        buffer.push_back(sib_byte);

    for (int i = 0; i < displacement_size; i++) {
        buffer.push_back(((char*)&displacement)[i]);
    }
}

void CommandEncoder::write_extended_opcode(char opcode, char extension, CommandParameter reg) {
    assert(reg.type == PARAMETER_TYPE_REG_64 || reg.type == PARAMETER_TYPE_REG_8);

    RegisterExtensions extensions = {.w = reg.type == PARAMETER_TYPE_REG_64};
    if (reg.reg > 7)
        extensions.b = true;

    char opcode_extension = 0b11000000;
    opcode_extension |= extension << 3;
    opcode_extension |= reg.reg;

    if (extensions.exist())
        buffer.push_back(extensions.get_byte());
    buffer.push_back(opcode);
    buffer.push_back(opcode_extension);
}

void CommandEncoder::write_extended_longopcode(char opcode_a, char opcode_b, char extension,
                                                CommandParameter reg) {
    assert(reg.type == PARAMETER_TYPE_REG_64 || reg.type == PARAMETER_TYPE_REG_8);

    RegisterExtensions extensions = {.w = reg.type == PARAMETER_TYPE_REG_64};
    if (reg.reg > 7)
        extensions.b = true;

    char opcode_extension = 0b11000000;
    opcode_extension |= extension << 3;
    opcode_extension |= reg.reg;

    if (extensions.exist())
        buffer.push_back(extensions.get_byte());
    buffer.push_back(opcode_a);
    buffer.push_back(opcode_b);
    buffer.push_back(opcode_extension);
}

uint64_t CommandEncoder::get_displacement(CommandParameter rm) {
    assert(rm.type == PARAMETER_TYPE_REG_64 || rm.type == PARAMETER_TYPE_REG_8 ||
           rm.type == PARAMETER_TYPE_REG_8 || rm.type == PARAMETER_TYPE_MEMORY ||
           rm.type == PARAMETER_TYPE_IMM32 || rm.type == PARAMETER_TYPE_SYMBOL);

    if (rm.type == PARAMETER_TYPE_REG_64 || rm.type == PARAMETER_TYPE_REG_8)
        return 0;
    if (rm.type == PARAMETER_TYPE_MEMORY)
        return rm.memory.displacement;
    return rm.imm;
}

MachineRegister CommandEncoder::to_machine_register(AbstractRegister reg) {
    // Assuming that the colorizer has already done its job,
    // the register should be a machine register (lay in a range of 0-15
    // and match the machine register index that it's mapped to)
    return (MachineRegister)reg;
}

} // namespace bonk::x86_backend