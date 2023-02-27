
#include "x86_colorizer_state_pop_context.hpp"
#include "x86_colorizer.hpp"

namespace bonk::x86_backend {

ColorizerStatePopContext::ColorizerStatePopContext(RegisterColorizer* the_colorizer,
                                                         int the_states_to_pop) {
    colorizer = the_colorizer;
    states_to_pop = the_states_to_pop;

    COLORIZER_DEBUG("restoring %d states\n", states_to_pop);
    count_transform();
}

bool ColorizerStatePopContext::count_transform() {
    auto stack = &colorizer->state_stack;
    int pop = 0;
    for (long long i = stack->size() - 1; pop < states_to_pop; i--, pop++) {
        auto& state = (*stack)[i];
        auto list = state.source;
        auto single_transform = state.relocation_info;

        // Joining transform map to our target transform

        for (auto it = single_transform.begin(); it != single_transform.end(); ++it) {
            target_transform.insert({it->first, it->second});
        }

        // Removing registers we've owned in current state

        for (auto it = target_transform.begin(); it != target_transform.end();) {
            if (it->second.owner == list) {
                it = target_transform.erase(it);
                continue;
            }
            ++it;
        }
    }

#ifdef COLORIZER_DEBUG_ON
    for (auto it = target_transform.begin(); it != target_transform.end(); ++it) {
        printf("Should restore _r%lld ", it->first);
        if (it->second.located_in_register)
            printf("to %s, ", ASM_REGISTERS_64[it->second.last_register_location]);
        if (it->second.located_in_memory)
            printf("to memory, ");
        auto descriptor = get_current_descriptor(it->first);
        if (descriptor.located_in_register)
            printf("now in %s\n", ASM_REGISTERS_64[descriptor.last_register_location]);
        if (descriptor.located_in_memory)
            printf("now in memory\n");
    }
#endif

    return true;
}

void ColorizerStatePopContext::walk_register_location_graph(bool* is_walked,
                                                               AbstractRegister i) {
    auto descriptor = get_current_descriptor(i);
    if (!should_save_register_with_owner(descriptor.owner)) {
        colorizer->release_register_location(i, true);
        return;
    }

    auto old_state = get_target_descriptor(i);

    if (!old_state.located_in_register) {
        colorizer->release_register_location(i, true);
        return;
    }

    MachineRegister current_location = descriptor.last_register_location;
    MachineRegister old_location = old_state.last_register_location;

    if (current_location == old_location) {
        // Everything is already on place
        return;
    }

    if (is_walked[current_location]) {
        // This situation happens when there is a circuit in location graph.
        // i.e, if _r1 and _r2 are used to be stored in rax and rbx, but
        // after some block their registers switched places (so they are
        // now stored in rbx, rax.
        // This can be resolved by exchanging places of last two
        // registers or by moving current register to temporary place
        // (which is more efficient)

        colorizer->exchange_registers(i, colorizer->machine_register_map[old_location]);

        return;
    }

    is_walked[current_location] = true;

    while (true) {
        // What is stored on old place? Can we just move our register to the old place?
        AbstractRegister bothering_register = colorizer->machine_register_map[old_location];
        if (bothering_register != -1 && bothering_register != i) {
            // No, we can't. We should move bothering register out of our way.
            walk_register_location_graph(is_walked, bothering_register);
            // However, this does not guarantee that this register will be available.
            // We must ensure that it's available, that's why this code is
            // placed inside a cycle.
        } else {
            break;
        }
    }

    is_walked[current_location] = false;

    // Updating descriptor as it may have been changed

    descriptor = get_current_descriptor(i);
    if (descriptor.last_register_location != old_location) {
        // If there is a cycle in the location graph,
        // the above while() cycle will already
        // restore location of current register, so
        // we don't want to waste mov instruction for
        // this
        assert(colorizer->gather_unused_register(old_location));
        colorizer->move_register_to_register(i, old_location);
    }
}

void ColorizerStatePopContext::try_restore_unowned_register_position(AbstractRegister i) {
    bool is_walked[16] = {};

    walk_register_location_graph(is_walked, i);
}

void ColorizerStatePopContext::restore_register_from_symbol(AbstractRegister reg,
                                                            MachineRegister target) {
    AbstractRegister used_register = colorizer->machine_register_map[target];

    if (used_register != -1) {
        colorizer->release_register_location(used_register, true);
    }

    assert(colorizer->gather_unused_register(target));
    colorizer->restore_register_from_symbol(reg, target);
}

void ColorizerStatePopContext::restore_register_from_memory(AbstractRegister reg,
                                                            MachineRegister target) {
    AbstractRegister used_register = colorizer->machine_register_map[target];

    if (used_register != -1) {
        colorizer->release_register_location(used_register, true);
    }

    assert(colorizer->gather_unused_register(target));
    colorizer->restore_register_from_stack(reg, target);
}

void ColorizerStatePopContext::restore_register_state(AbstractRegister reg,
                                                         AbstractRegisterDescriptor old_state,
                                                      StateRegisterRestoreStage stage) {
    auto current_state = get_current_descriptor(reg);

    if (old_state.located_in_memory) {
        if (current_state.located_in_register) {
            // First step: write all loaded registers back
            if (stage == RESTORE_STAGE_MEM_REG) {
                COLORIZER_DEBUG("Restoring _r%lld (reg -> mem)\n", reg);
                colorizer->move_register_to_stack(reg, current_state.last_memory_position);
            }
        } else if (current_state.located_in_memory) {
            // Do nothing, probably
        } else {
            // Just locate register at its place. We have probably
            // deleted in while popping states somewhere

            colorizer->locate_register_at_stack(reg, old_state.last_memory_position);
        }
    } else if (old_state.located_in_register) {
        if (current_state.located_in_register) {
            // Second step: shuffle all register-stored registers
            if (stage == RESTORE_STAGE_REG_REG) {
                COLORIZER_DEBUG("Restoring _r%lld (reg -> reg)\n", reg);
                try_restore_unowned_register_position(reg);
            }
        } else if (current_state.located_in_memory) {
            if (stage == RESTORE_STAGE_REG_MEM) {
                COLORIZER_DEBUG("Restoring _r%lld (mem -> reg)\n", reg);
                // Third step:
                // We guarantee that old_state.last_register_location is unused for now,
                // since for now each register-stored register is holding its
                // original place on this step
                restore_register_from_memory(reg, old_state.last_register_location);
            }
        } else if (current_state.located_in_symbol) {
            if (stage == RESTORE_STAGE_REG_MEM) {
                restore_register_from_symbol(reg, old_state.last_register_location);
            }
        } else {
            // Just locate register at its place. We have probably
            // deleted in while popping states somewhere

            colorizer->locate_register_at_machine_register(reg, old_state.last_register_location,
                                                           nullptr);
        }
    } else if (old_state.located_in_symbol) {
        if (current_state.located_in_register) {
            if (stage == RESTORE_STAGE_MEM_REG) {
                colorizer->move_register_to_symbol(reg);
            }
        } else if (!current_state.located_in_symbol) {
            colorizer->locate_register_at_symbol(reg);
        }
    }
}

void ColorizerStatePopContext::restore_registers() {
    for (int stage = 0; stage < 3; stage++) {
        for (auto it = target_transform.begin(); it != target_transform.end(); ++it) {
            restore_register_state(it->first, it->second, StateRegisterRestoreStage(stage));
        }
    }
}

AbstractRegisterDescriptor ColorizerStatePopContext::get_current_descriptor(AbstractRegister reg) {
    return *colorizer->source->descriptors.get_descriptor(reg);
}

AbstractRegisterDescriptor ColorizerStatePopContext::get_target_descriptor(AbstractRegister reg) {
    auto it = target_transform.find(reg);
    if (it != target_transform.end())
        return it->second;
    return *colorizer->source->descriptors.get_descriptor(reg);
}

bool ColorizerStatePopContext::should_save_register_with_owner(CommandList* owner) {
    auto& stack = colorizer->state_stack;
    int pop = 0;
    for (long long i = stack.size() - 1; pop < states_to_pop; i--, pop++) {
        if (stack[i].source == owner)
            return false;
    }
    return true;
}

} // namespace bonk::x86_backend