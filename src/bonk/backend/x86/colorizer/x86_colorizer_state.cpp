
#include "x86_colorizer_state.hpp"
#include "x86_colorizer.hpp"

namespace bonk::x86_backend {

register_colorize_context_state::register_colorize_context_state(
    command_list* list, register_colorize_context_state* old_state) {
    source = list;
    create_usage_heap_array();
}

void register_colorize_context_state::add_register_usage(abstract_register reg,
                                                         abstract_register_usage usage) {
    auto it = usage_heap_array.find(reg);

    if (it == usage_heap_array.end()) {
        auto new_usage_heap = new bin_heap<abstract_register_usage>();
        new_usage_heap->push(usage);
        usage_heap_array.insert({reg, new_usage_heap});
    } else {
        it->second->push(usage);
    }
}

bool register_colorize_context_state::create_usage_heap_array() {
    int instruction_index = 0;

    for (auto i = source->begin(); i != source->end(); source->next_iterator(&i)) {
        asm_command* command = source->get(i);

        if (command->type == COMMAND_COLORIZER_REPEAT_SCOPE) {
            duplicate_usages();
            // "repeat scope" should be last command in the scope.
            assert(source->next_iterator(i) == source->end());
        }

        for (auto reg : command->read_registers) {
            add_register_usage(reg, {(long)instruction_index, false});
        }

        for (auto reg : command->write_registers) {
            add_register_usage(reg, {(long)instruction_index, true});
        }

        instruction_index++;
    }

    return true;
}

void register_colorize_context_state::duplicate_usages() {

    for (auto i = usage_heap_array.begin(); i != usage_heap_array.end(); i++) {
        auto& key = i->first;
        auto& usage_heap = i->second;

        if (source->parent_buffer->descriptors.get_descriptor(key)->owner != source) {
            usage_heap->push({(long)(source->get_size() + 1), false});
        }
    }
}

} // namespace bonk::x86_backend