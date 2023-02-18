#pragma once

#include <unordered_map>
#include "../x86_abstract_register.hpp"

namespace bonk::x86_backend {

struct colorizer_state_pop_context;
struct register_colorizer;

enum state_register_restore_stage {
    RESTORE_STAGE_MEM_REG,
    RESTORE_STAGE_REG_REG,
    RESTORE_STAGE_REG_MEM
};

} // namespace bonk::x86_backend

namespace bonk::x86_backend {

struct colorizer_state_pop_context {
    register_colorizer* colorizer;
    int states_to_pop;

    std::unordered_map<abstract_register, abstract_register_descriptor> target_transform;

    colorizer_state_pop_context(register_colorizer* the_colorizer, int the_states_to_pop);

    bool count_transform();

    bool should_save_register_with_owner(command_list* owner);

    abstract_register_descriptor get_current_descriptor(abstract_register reg);

    abstract_register_descriptor get_target_descriptor(abstract_register reg);

    void restore_registers();

    void restore_register_state(abstract_register reg, abstract_register_descriptor old_state,
                                state_register_restore_stage stage);

    void walk_register_location_graph(bool* is_walked, abstract_register i);

    void try_restore_unowned_register_position(abstract_register i);

    void restore_register_from_memory(abstract_register reg, e_machine_register target);

    void restore_register_from_symbol(abstract_register reg, e_machine_register aRegister);
};

} // namespace bonk::x86_backend
