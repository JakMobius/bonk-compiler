#pragma once

namespace bonk::x86_backend {

struct EmplaceRequest;
struct CommandBuffer;
struct JmpLabel;
struct AsmCommand;
struct CommandParameter;
} // namespace bonk::x86_backend

#include <vector>
#include "../../target/macho/macho.hpp"
#include "x86_abstract_register.hpp"
#include "x86_machine_register.hpp"
#include "x86_register_extensions.hpp"

namespace bonk::x86_backend {

struct EmplaceRequest {
    unsigned long offset;
    unsigned long relation;
    char bytes;
    JmpLabel* command;
};

struct RelocationRequest {
    uint32_t relocation;
    int32_t address;
    macho::RelocationType type;
    uint8_t data_length;
    bool pc_rel;
};

struct CommandEncoder {
    std::vector<EmplaceRequest> emplace_requests;
    std::vector<RelocationRequest> relocation_requests;
    std::vector<char> buffer;

    CommandEncoder();

    void request_emplace(EmplaceRequest request);

    void do_emplacements();

    void request_relocation(RelocationRequest request);

    bool is_sip(CommandParameter reg_rm);

    char get_displacement_bytes(CommandParameter reg_rm, RegisterExtensions* extensions);

    char get_mod_reg_rm_byte(CommandParameter reg, CommandParameter reg_rm,
                             RegisterExtensions* extensions);

    char get_sib_byte(CommandParameter reg_rm, RegisterExtensions* extensions);

    uint64_t get_displacement(CommandParameter rm);

    void get_sib_base_index(CommandParameter reg_rm, char* index_p, char* base_p,
                            bool* requires_mask);

    void write_prefix_opcode_modrm_sib(char opcode, CommandParameter reg, CommandParameter reg_rm);

    void write_prefix_longopcode_regrm_sib(char opcode_a, char opcode_b, CommandParameter reg,
                                           CommandParameter reg_rm);

    void write_extended_opcode(char opcode, char extension, CommandParameter reg);

    void write_extended_longopcode(char opcode_a, char opcode_b, char extension,
                                   CommandParameter reg);

    MachineRegister to_machine_register(AbstractRegister reg);
};

} // namespace bonk::x86_backend
