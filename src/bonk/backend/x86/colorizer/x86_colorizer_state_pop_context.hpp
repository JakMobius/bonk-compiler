#pragma once

#include <unordered_map>
#include "../x86_abstract_register.hpp"

namespace bonk::x86_backend {

struct ColorizerStatePopContext;
struct RegisterColorizer;

enum StateRegisterRestoreStage {
    RESTORE_STAGE_MEM_REG,
    RESTORE_STAGE_REG_REG,
    RESTORE_STAGE_REG_MEM
};

} // namespace bonk::x86_backend

namespace bonk::x86_backend {

struct ColorizerStatePopContext {
    RegisterColorizer* colorizer;
    int states_to_pop;

    std::unordered_map<AbstractRegister, AbstractRegisterDescriptor> target_transform;

    ColorizerStatePopContext(RegisterColorizer* the_colorizer, int the_states_to_pop);

    bool count_transform();

    bool should_save_register_with_owner(CommandList* owner);

    AbstractRegisterDescriptor get_current_descriptor(AbstractRegister reg);

    AbstractRegisterDescriptor get_target_descriptor(AbstractRegister reg);

    void restore_registers();

    void restore_register_state(AbstractRegister reg, AbstractRegisterDescriptor old_state,
                                StateRegisterRestoreStage stage);

    void walk_register_location_graph(bool* is_walked, AbstractRegister i);

    void try_restore_unowned_register_position(AbstractRegister i);

    void restore_register_from_memory(AbstractRegister reg, MachineRegister target);

    void restore_register_from_symbol(AbstractRegister reg, MachineRegister aRegister);
};

} // namespace bonk::x86_backend
