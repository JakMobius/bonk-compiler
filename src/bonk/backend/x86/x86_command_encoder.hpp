#pragma once

namespace bonk::x86_backend {

struct emplace_request;
struct command_buffer;
struct jmp_label;
struct asm_command;
struct command_parameter;
} // namespace bonk::x86_backend

#include <vector>
#include "../../target/macho/macho.hpp"
#include "x86_abstract_register.hpp"
#include "x86_machine_register.hpp"
#include "x86_register_extensions.hpp"

namespace bonk::x86_backend {

struct emplace_request {
    unsigned long offset;
    unsigned long relation;
    char bytes;
    jmp_label* command;
};

struct relocation_request {
    uint32_t relocation;
    int32_t address;
    macho::relocation_type type;
    uint8_t data_length;
    bool pc_rel;
};

struct command_encoder {
    std::vector<emplace_request> emplace_requests;
    std::vector<relocation_request> relocation_requests;
    std::vector<char> buffer;

    command_encoder();

    void request_emplace(emplace_request request);

    void do_emplacements();

    void request_relocation(relocation_request request);

    bool is_sip(command_parameter reg_rm);

    char get_displacement_bytes(command_parameter reg_rm, register_extensions* extensions);

    char get_mod_reg_rm_byte(command_parameter reg, command_parameter reg_rm,
                             register_extensions* extensions);

    char get_sib_byte(command_parameter reg_rm, register_extensions* extensions);

    uint64_t get_displacement(command_parameter rm);

    void get_sib_base_index(command_parameter reg_rm, char* index_p, char* base_p,
                            bool* requires_mask);

    void write_prefix_opcode_modrm_sib(char opcode, command_parameter reg,
                                       command_parameter reg_rm);

    void write_prefix_longopcode_regrm_sib(char opcode_a, char opcode_b, command_parameter reg,
                                           command_parameter reg_rm);

    void write_extended_opcode(char opcode, char extension, command_parameter reg);

    void write_extended_longopcode(char opcode_a, char opcode_b, char extension,
                                   command_parameter reg);

    e_machine_register to_machine_register(abstract_register reg);
};

} // namespace bonk::x86_backend
