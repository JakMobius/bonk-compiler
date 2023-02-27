
#include "x86_command_buffer.hpp"
#include "instructions/x86_add.hpp"
#include "instructions/x86_colorizer_align_stack.hpp"
#include "instructions/x86_colorizer_dead_end.hpp"
#include "instructions/x86_colorizer_locate_reg.hpp"
#include "instructions/x86_colorizer_reg_preserve.hpp"
#include "instructions/x86_colorizer_scope.hpp"
#include "instructions/x86_conditional_instructions.hpp"
#include "instructions/x86_idiv.hpp"
#include "instructions/x86_imul.hpp"
#include "instructions/x86_mov.hpp"
#include "instructions/x86_push.hpp"
#include "instructions/x86_sub.hpp"
#include "instructions/x86_xor.hpp"

namespace bonk::x86_backend {

CommandBuffer::CommandBuffer(macho::MachoFile* file) {
    this->file = file;
    allocator.set_page_capacity(4096);
    root_list = next_command_list();
}

CommandBuffer::CommandBuffer(RegisterDescriptorList* descriptor_list, macho::MachoFile* file) {
    this->file = file;
    allocator.set_page_capacity(4096);
    root_list = next_command_list();
}

CommandBuffer::~CommandBuffer() {
    for (int i = 0; i < lists.size(); i++) {
        delete lists[i];
    }
}

CommandList* CommandBuffer::next_command_list() {
    auto* next_list = new CommandList(this);
    lists.push_back(next_list);
    return next_list;
}

CommandEncoder* CommandBuffer::to_bytes() {
    auto* encoder = new CommandEncoder();

    for (auto i = root_list->commands.begin(); i != root_list->commands.end(); ++i) {
        AsmCommand* command = *i;
        command->offset = encoder->buffer.size();
        command->to_bytes(encoder);
    }

    return encoder;
}

void CommandBuffer::write_block_to_object_file(const std::string& block_name,
                                                macho::MachoFile* target) {

    auto encoder = to_bytes();
    auto offset = target->section_text.size;

    encoder->do_emplacements();
    for (int i = 0; i < encoder->relocation_requests.size(); i++) {
        auto& relocation_request = encoder->relocation_requests[i];

        target->add_relocation(relocation_request.relocation, relocation_request.address + offset,
                               relocation_request.pc_rel, relocation_request.data_length);
    }

    target->add_internal_symbol(block_name, macho::SYMBOL_SECTION_TEXT, offset);
    target->add_code({encoder->buffer.data(), encoder->buffer.size()});
}

void CommandList::append_read_register(std::set<AbstractRegister>* tree) {
    for (auto i = commands.begin(); i != commands.end(); ++i) {
        AsmCommand* command = *i;
        for (int j = 0; j < command->read_registers.size(); j++) {
            auto reg = command->read_registers[j];
            if (parent_buffer->descriptors.get_descriptor(reg)->owner == this)
                continue;
            tree->insert(reg);
        }
    }
}

void CommandList::append_write_register(std::set<AbstractRegister>* tree) {
    for (auto i = commands.begin(); i != commands.end(); i = ++i) {
        AsmCommand* command = *i;
        for (int j = 0; j < command->read_registers.size(); j++) {
            auto reg = command->read_registers[j];
            if (parent_buffer->descriptors.get_descriptor(reg)->owner == this)
                continue;
            tree->insert(reg);
        }
    }
}

CommandList::CommandList(CommandBuffer* parent_buffer) {
    this->parent_buffer = parent_buffer;
}
} // namespace bonk::x86_backend