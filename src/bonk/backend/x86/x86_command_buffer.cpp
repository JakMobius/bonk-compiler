
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

command_buffer::command_buffer(macho::macho_file* file) {
    this->file = file;
    allocator.set_page_capacity(4096);
    root_list = next_command_list();
}

command_buffer::command_buffer(register_descriptor_list* descriptor_list, macho::macho_file* file) {
    this->file = file;
    allocator.set_page_capacity(4096);
    root_list = next_command_list();
}

command_buffer::~command_buffer() {
    for (int i = 0; i < lists.size(); i++) {
        delete lists[i];
    }
}

command_list* command_buffer::next_command_list() {
    auto* next_list = new command_list(this);
    lists.push_back(next_list);
    return next_list;
}

command_encoder* command_buffer::to_bytes() {
    command_encoder* encoder = new command_encoder();

    for (auto i = root_list->begin(); i != root_list->end(); root_list->next_iterator(&i)) {
        asm_command* command = root_list->get(i);
        command->offset = encoder->buffer.size();
        command->to_bytes(encoder);
    }

    return encoder;
}

void command_buffer::write_block_to_object_file(const std::string& block_name,
                                                macho::macho_file* target) {

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

void command_list::append_read_register(std::set<abstract_register>* tree) {
    for (auto i = begin(); i != end(); next_iterator(&i)) {
        asm_command* command = get(i);
        for (int j = 0; j < command->read_registers.size(); j++) {
            auto reg = command->read_registers[j];
            if (parent_buffer->descriptors.get_descriptor(reg)->owner == this)
                continue;
            tree->insert(reg);
        }
    }
}

void command_list::append_write_register(std::set<abstract_register>* tree) {
    for (auto i = begin(); i != end(); i = next_iterator(&i)) {
        asm_command* command = get(i);
        for (int j = 0; j < command->read_registers.size(); j++) {
            auto reg = command->read_registers[j];
            if (parent_buffer->descriptors.get_descriptor(reg)->owner == this)
                continue;
            tree->insert(reg);
        }
    }
}

command_list::command_list(command_buffer* parent_buffer) {
    this->parent_buffer = parent_buffer;
}
} // namespace bonk::x86_backend