
#include "x86_colorizer_state.hpp"
#include "x86_colorizer.hpp"

namespace bonk::x86_backend {

RegisterColorizeContextState::RegisterColorizeContextState(
    CommandList* list, RegisterColorizeContextState* old_state) {
    source = list;
    create_usage_heap_array();
}

void RegisterColorizeContextState::add_register_usage(AbstractRegister reg,
                                                      AbstractRegisterUsage usage) {
    auto it = usage_heap_array.find(reg);

    if (it == usage_heap_array.end()) {
        auto new_usage_heap = new bin_heap<AbstractRegisterUsage>();
        new_usage_heap->push(usage);
        usage_heap_array.insert({reg, new_usage_heap});
    } else {
        it->second->push(usage);
    }
}

bool RegisterColorizeContextState::create_usage_heap_array() {
    int instruction_index = 0;

    for (AsmCommand* command : source->commands) {
        if (command->type == COMMAND_COLORIZER_REPEAT_SCOPE) {
            duplicate_usages();
            // "repeat scope" should be last command in the scope.
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

void RegisterColorizeContextState::duplicate_usages() {

    for (auto & i : usage_heap_array) {
        auto& key = i.first;
        auto& usage_heap = i.second;

        if (source->parent_buffer->descriptors.get_descriptor(key)->owner != source) {
            usage_heap->push({(long)(source->commands.size() + 1), false});
        }
    }
}

} // namespace bonk::x86_backend