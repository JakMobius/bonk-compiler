#pragma once

namespace bonk::x86_backend {

struct command_buffer;
struct command_list;

} // namespace bonk::x86_backend

#include <set>
#include "../../../utils/linear_allocator.hpp"
#include "../../target/macho/macho.hpp"
#include "instructions/x86_command.hpp"
#include "x86_command_encoder.hpp"
#include "x86_register_descriptor_list.hpp"

namespace bonk::x86_backend {

struct command_list : mlist<asm_command*> {
    command_buffer* parent_buffer;

    command_list(command_buffer* parent_buffer);

    void append_read_register(std::set<abstract_register>* tree);

    void append_write_register(std::set<abstract_register>* tree);
};

struct command_buffer {
    macho::macho_file* file;
    command_list* root_list;
    linear_allocator allocator;
    register_descriptor_list descriptors;
    std::vector<command_list*> lists;
    std::vector<command_list*> list;
    unsigned long labels;

    // Creates completely new independent command parent_buffer

    command_buffer(macho::macho_file* file = nullptr);

    // Creates command buffer which will use the same register namespace as
    // another command buffer

    command_buffer(register_descriptor_list* descriptor_list, macho::macho_file* file = nullptr);

    ~command_buffer();

    command_list* next_command_list();

    void write_block_to_object_file(const std::string& block_name, macho::macho_file* target);

    command_encoder* to_bytes();
};

} // namespace bonk::x86_backend
