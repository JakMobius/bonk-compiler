#pragma once

namespace bonk::x86_backend {

struct CommandBuffer;
struct CommandList;

} // namespace bonk::x86_backend

#include <set>
#include "../../../utils/linear_allocator.hpp"
#include "../../target/macho/macho.hpp"
#include "instructions/x86_command.hpp"
#include "x86_command_encoder.hpp"
#include "x86_register_descriptor_list.hpp"

namespace bonk::x86_backend {

struct CommandList {
    CommandBuffer* parent_buffer {};
    std::list<AsmCommand*> commands {};

    CommandList(CommandBuffer* parent_buffer);

    void append_read_register(std::set<AbstractRegister>* tree);
    void append_write_register(std::set<AbstractRegister>* tree);
};

struct CommandBuffer {
    macho::MachoFile* file = nullptr;
    CommandList* root_list = nullptr;
    LinearAllocator allocator {};
    RegisterDescriptorList descriptors {};
    std::vector<CommandList*> lists {};
    std::vector<CommandList*> list {};
    unsigned long labels = 0;

    // Creates completely new independent command parent_buffer

    CommandBuffer(macho::MachoFile* file = nullptr);

    // Creates command buffer which will use the same register namespace as
    // another command buffer

    CommandBuffer(RegisterDescriptorList* descriptor_list, macho::MachoFile* file = nullptr);

    ~CommandBuffer();

    CommandList* next_command_list();

    void write_block_to_object_file(const std::string& block_name, macho::MachoFile* target);

    CommandEncoder* to_bytes();
};

} // namespace bonk::x86_backend
