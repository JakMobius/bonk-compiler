
#include "x86_colorizer.hpp"

namespace bonk::x86_backend {

unsigned long long colorizer_command_value(JmpLabel* value) {
    return (unsigned long long)value;
}

bool colorizer_command_comparator(JmpLabel* a, JmpLabel* b) {
    return a == b;
}

const MachineRegister SYSTEM_V_OPTIMAL_REGISTERS[] = {
    // Callee-preserved, should be used last
    r15, r14, r13, r12, rbx,

    // Caller-preserved, should be used first
    r11, r10, r9, r8, rdi, rsi, rdx, rcx, rax};

RegisterColorizer::RegisterColorizer(CommandBuffer* input) {
    state = nullptr;
    source = input;

    for (auto i : SYSTEM_V_OPTIMAL_REGISTERS)
        unused_registers_stack.push_back(i);
    machine_register_map = (AbstractRegister*)(calloc(16, sizeof(AbstractRegister)));
    for (int i = 0; i < 16; i++)
        machine_register_map[i] = -1;
}

CommandBuffer* RegisterColorizer::colorize(CommandBuffer* commands) {
    RegisterColorizer ctx{commands};
    ctx.output = new CommandBuffer(&ctx.source->descriptors, ctx.source->file);

    ctx.push_state(commands->root_list);
    ctx.run_colorizer();
    ctx.replace_frame_commands();
    ctx.pop_state();

    ctx.transform_labels();

    return ctx.output;
}

RegisterColorizer::~RegisterColorizer() {
    frame_size = 0;
}

void RegisterColorizer::save_register_location(AbstractRegister reg) {
    assert(reg != -1);

    auto* descriptor = source->descriptors.get_descriptor(reg);

    // We should only save state of unowned registers
    if (descriptor->owner == state->source)
        return;

    if (state->relocation_info.find(reg) == state->relocation_info.end()) {
        state->relocation_info.insert({reg, *descriptor});
    }
}

void RegisterColorizer::move_register_to_register(AbstractRegister reg, MachineRegister target) {
    assert(reg != -1 && target != rinvalid);
    save_register_location(reg);
    auto* descriptor = source->descriptors.get_descriptor(reg);

    // Register should have already been gathered, but should be unclaimed
    assert(!is_unused_register(target));
    assert(machine_register_map[target] == -1);

    output->root_list->commands.push_back(new MovCommand(
        CommandParameter::create_register_64(output->descriptors.machine_register(target)),
        CommandParameter::create_register_64(
            output->descriptors.machine_register(descriptor->last_register_location))));

    if (descriptor->located_in_register) {
        release_register_immediately(descriptor->last_register_location, false);
    }

    descriptor->located_in_register = true;
    descriptor->last_register_location = target;
    machine_register_map[descriptor->last_register_location] = reg;
}

void RegisterColorizer::move_register_to_stack(AbstractRegister reg, long long frame_position) {
    assert(reg != -1);
    save_register_location(reg);

    auto* descriptor = source->descriptors.get_descriptor(reg);

    assert(descriptor->located_in_register);

    output->root_list->commands.push_back(
        new MovCommand(CommandParameter::create_memory(CommandParameterMemory::create_reg_displ(
                            output->descriptors.machine_register(rbp), -frame_position)),
                        CommandParameter::create_register_64(output->descriptors.machine_register(
                            descriptor->last_register_location))));

    release_register_immediately(descriptor->last_register_location, false);

    descriptor->last_memory_position = frame_position;
    descriptor->located_in_memory = true;
    descriptor->located_in_register = false;
}

void RegisterColorizer::restore_register_from_stack(AbstractRegister reg, MachineRegister target) {
    assert(reg != -1 && target != rinvalid);
    save_register_location(reg);

    auto* descriptor = source->descriptors.get_descriptor(reg);
    assert(descriptor->located_in_memory && !descriptor->located_in_register);

    output->root_list->commands.push_back(new MovCommand(
        CommandParameter::create_register_64(output->descriptors.machine_register(target)),
        CommandParameter::create_memory(CommandParameterMemory::create_reg_displ(
            output->descriptors.machine_register(rbp), -descriptor->last_memory_position))));

    if (descriptor->owner == state->source) {
        release_frame_cell(descriptor->last_memory_position);
    }

    descriptor->located_in_memory = false;
    descriptor->located_in_register = true;
    descriptor->last_register_location = target;

    machine_register_map[descriptor->last_register_location] = reg;

    COLORIZER_DEBUG(";  now register is stored in %s\n",
                    ASM_REGISTERS_64[descriptor->last_register_location]);
}

void RegisterColorizer::restore_register(AbstractRegister reg, MachineRegister target) {
    assert(reg != -1 && target != rinvalid);
    save_register_location(reg);

    auto* descriptor = source->descriptors.get_descriptor(reg);
    // Register should have already been gathered, but should be unclaimed
    // assert(!is_unused_register(target));
    assert(machine_register_map[target] == -1);

    if (descriptor->located_in_memory && descriptor->owner == state->source) {
        release_frame_cell(descriptor->last_memory_position);
    }

    descriptor->located_in_memory = false;
    descriptor->located_in_register = true;
    descriptor->last_register_location = target;

    machine_register_map[descriptor->last_register_location] = reg;

    COLORIZER_DEBUG(";  now register is stored in %s\n",
                    ASM_REGISTERS_64[descriptor->last_register_location]);
}

int RegisterColorizer::frame_cell_for_register(AbstractRegister reg) {
    auto* descriptor = source->descriptors.get_descriptor(reg);

    auto it = state->relocation_info.find(reg);

    if (descriptor->owner != state->source && it != state->relocation_info.end()) {
        auto& old_descriptor = it->second;
        if (old_descriptor.located_in_memory)
            return old_descriptor.last_memory_position;
    }

    return retain_frame_cell();
}

void RegisterColorizer::move_register(AbstractRegister reg, AsmCommand* command) {
    assert(reg != -1);
    save_register_location(reg);

    auto* descriptor = source->descriptors.get_descriptor(reg);
    // Register should have already been gathered, but should be unclaimed
    assert(descriptor->located_in_register);

    auto usage_heap = get_register_usage_heap(reg);

    // If there are no usages (this check is suitable for global variables)
    if (!usage_heap || !usage_heap->length) {
        release_register_location(reg, true);
        COLORIZER_DEBUG(
            ";  _r%lld value may not be saved as it is used last time in current operation\n", reg);
    } else if (usage_heap->get_min().is_write) {
        release_register_location(reg, true);
        COLORIZER_DEBUG(
            ";  _r%lld value may not be saved as current register value will never be read\n", reg);
    } else {
        /* Here we must ignore registers that we've already used in current command
         because there could be such situation:

         mov rcx, 1
         ; colorizer will try to save rdi here if it's used later
         ; it will choose rcx register and insert "mov rcx, rdi" here,
         ; as rcx turns to be unused after processing input registers of mov
         mov rdi, rcx
         call func
        */
        MachineRegister register_to_move = effective_unused_register(command, true);

        if (register_to_move == rinvalid) {
            if (descriptor->has_symbol_position) {
                move_register_to_symbol(reg);
            } else {
                COLORIZER_DEBUG(";  moving to memory _r%lld and releasing %s\n", reg,
                                ASM_REGISTERS_64[descriptor->last_register_location]);

                move_register_to_stack(reg, frame_cell_for_register(reg));
            }
        } else {
            COLORIZER_DEBUG(";  moving _r%lld from %s to %s\n", reg,
                            ASM_REGISTERS_64[descriptor->last_register_location],
                            ASM_REGISTERS_64[register_to_move]);

            move_register_to_register(reg, register_to_move);
        }
    }
}

void RegisterColorizer::exchange_registers(AbstractRegister reg_a, AbstractRegister reg_b) {
    auto descriptor_a = source->descriptors.get_descriptor(reg_a);
    auto descriptor_b = source->descriptors.get_descriptor(reg_b);
    save_register_location(reg_a);
    save_register_location(reg_b);

    output->root_list->commands.push_back(new XchgCommand(
        CommandParameter::create_register_64(
            output->descriptors.machine_register(descriptor_a->last_register_location)),
        CommandParameter::create_register_64(
            output->descriptors.machine_register(descriptor_b->last_register_location))));

    MachineRegister temp = descriptor_a->last_register_location;
    descriptor_a->last_register_location = descriptor_b->last_register_location;
    descriptor_b->last_register_location = temp;

    machine_register_map[descriptor_a->last_register_location] =
        descriptor_a->last_register_location;
    machine_register_map[descriptor_b->last_register_location] =
        descriptor_b->last_register_location;
}

AbstractRegister RegisterColorizer::least_used_register_for_command(AsmCommand* command,
                                                                      bool is_write) {

    AbstractRegister result = -1;
    long long result_position = -1;
    long long result_scope = state_stack.size() - 1;

    for (int i = 0; i < 16; i++) {
        AbstractRegister placeholder = machine_register_map[i];

        // As unused registers are gathered in specific order,
        // with different algorithm, we should not check them
        // registers here
        if (placeholder == -1)
            continue;
        if (command_uses_register(command, MachineRegister(i), true))
            continue;

        bin_heap<AbstractRegisterUsage>* usage_info = nullptr;
        long long usage_scope = result_scope;

        for (; usage_scope >= 0; usage_scope--) {
            auto& state = state_stack[usage_scope];

            auto it = state.usage_heap_array.find(placeholder);

            if (it != state.usage_heap_array.end()) {
                usage_info = it->second;
                break;
            }
        }

        if (usage_info) {
            long long usage_index = LONG_LONG_MAX;
            if (usage_info->length) {
                usage_index = usage_info->get_min().instruction_index;
            }
            if (usage_scope < result_scope || usage_index > result_position) {
                result = placeholder;
                result_position = usage_index;
                result_scope = usage_scope;
            }
        }
    }

    return result;
}

void RegisterColorizer::handle_register_use(AsmCommand* command, AbstractRegister reg,
                                             bool is_write) {
    COLORIZER_DEBUG(";  _r%llu ", reg);

    auto* descriptor = source->descriptors.get_descriptor(reg);

    if (!descriptor->located_in_register) {
        pop_nearest_register_usage(reg);
        COLORIZER_DEBUG("is not loaded into register\n");
        if (descriptor->has_register_constraint) {
            COLORIZER_DEBUG(";  it's constrained to %s\n",
                            ASM_REGISTERS_64[descriptor->last_register_location]);

            if (!descriptor->located_in_register) {
                if (!is_write) {
                    // If it's constrained read access, treating it as
                    // simple value read. We should not change
                    // anything, as it may lead to duplicates
                    // in unused registers stack.
                    destroy_register_if_unused(reg);
                    return;
                }

                locate_register_at_machine_register(reg, descriptor->last_register_location,
                                                    command);
            }
        } else {

            // Here we must select a register to associate
            // it with abstract register which was moved
            // to memory before

            MachineRegister effective_register = effective_unused_register(command, false);

            if (effective_register == rinvalid) {
                AbstractRegister most_useless_register =
                    least_used_register_for_command(command, is_write);
                move_register(most_useless_register, command);
                effective_register = effective_unused_register(command, false);
            }

            if (descriptor->has_symbol_position) {
                if (descriptor->located_in_symbol && !is_write) {
                    restore_register_from_symbol(reg, effective_register);
                } else {
                    restore_register(reg, effective_register);
                }
            } else {
                if (descriptor->located_in_memory && !is_write) {
                    restore_register_from_stack(reg, effective_register);
                } else {
                    restore_register(reg, effective_register);
                }
            }
        }
        destroy_register_if_unused(reg);
    } else {
        COLORIZER_DEBUG("is already in %s\n", ASM_REGISTERS_64[descriptor->last_register_location]);
        pop_nearest_register_usage(reg);
        destroy_register_if_unused(reg);
    }
}

void RegisterColorizer::push_state(CommandList* list) {
    RegisterColorizeContextState* old_state = nullptr;
    if (state_stack.size() > 0)
        old_state = &state_stack[state_stack.size() - 1];
    state_stack.push_back({list, old_state});
    state = &state_stack[state_stack.size() - 1];
}

void RegisterColorizer::pop_state() {

    //#ifdef COLORIZER_DEBUG
    //    for(int i = 0; i < state->usage_heap_array.capacity; i++) {
    //        auto chain = &state->usage_heap_array.lists[i];
    //        for(auto j = chain->begin(); j != chain->end(); chain->next_iterator(&j)) {
    //            auto entry = chain->get(j);
    //            printf("ERROR: LEFT USAGE: _r%lld SIZE %lld\n", entry.key, entry.value->length);
    //        }
    //    }
    //    assert(state->usage_heap_array.size == 0);
    //#endif

    // pop_descriptors_state();

    state_stack.pop_back();
    if (state_stack.size()) {
        state = &state_stack[state_stack.size() - 1];
    } else {
        state = nullptr;
    }
}

AbstractRegister RegisterColorizer::get_colorized_register(AsmCommand* command,
                                                            AbstractRegister reg, bool is_write) {
    bool is_read_register = false;
    bool is_write_register = false;

    for (auto each_read_register : command->read_registers) {
        if (reg == each_read_register) {
            is_read_register = true;
            break;
        }
    }

    for (auto each_write_register : command->write_registers) {
        if (reg == each_write_register) {
            is_write_register = true;
            break;
        }
    }

    if ((is_read_register && !is_write_register && !is_write) || is_write) {
        auto last_location = source->descriptors.get_descriptor(reg)->last_register_location;
        return source->descriptors.machine_register(last_location);
    }

    return reg;
}

void RegisterColorizer::colorize_command(AsmCommand* command, bool is_write) {

    for (int i = 0; i < command->parameters.size(); i++) {
        CommandParameter parameter = command->parameters[i];

        if (parameter.type == PARAMETER_TYPE_REG_64) {
            AbstractRegister reg = get_colorized_register(command, parameter.reg, is_write);

            command->parameters[i] = CommandParameter::create_register_64(reg);
        } else if (parameter.type == PARAMETER_TYPE_REG_8) {
            AbstractRegister reg = get_colorized_register(command, parameter.reg, is_write);

            command->parameters[i] = CommandParameter::create_register_8(reg);
        } else if (parameter.type == PARAMETER_TYPE_MEMORY) {

            parameter.memory.reg_a =
                get_colorized_register(command, parameter.memory.reg_a, is_write);
            parameter.memory.reg_b =
                get_colorized_register(command, parameter.memory.reg_b, is_write);

            command->parameters[i] = parameter;
        }
    }

    if (is_write) {
        for (int i = 0; i < command->write_registers.size(); i++) {
            AbstractRegister reg = command->write_registers[i];
            auto last_location = source->descriptors.get_descriptor(reg)->last_register_location;
            command->write_registers[i] = source->descriptors.machine_register(last_location);
        }
    } else {
        for (int i = 0; i < command->read_registers.size(); i++) {
            AbstractRegister reg = command->read_registers[i];
            auto last_location = source->descriptors.get_descriptor(reg)->last_register_location;
            command->read_registers[i] = source->descriptors.machine_register(last_location);
        }
    }
}

long long RegisterColorizer::retain_frame_cell() {
    if (released_frame_cells.size()) {
        long long cell = released_frame_cells[released_frame_cells.size() - 1];
        released_frame_cells.pop_back();
        return cell;
    }
    frame_size += 8;
    return frame_size;
}

void RegisterColorizer::release_frame_cell(long long cell) {
    released_frame_cells.push_back(cell);
}

void RegisterColorizer::handle_reg_preserve_command(RegPreserveCommand* command) {
    for (auto read_register : command->read_registers) {
        handle_register_use(command, read_register, false);
    }
    flush_unused_registers();

    for (auto write_register : command->write_registers) {
        handle_register_use(command, write_register, true);
    }
    flush_unused_registers();
    registers_released_by_command.clear();
}

void RegisterColorizer::pop_scopes(int scopes) {
    ColorizerStatePopContext restore_context = {this, scopes};
    restore_context.restore_registers();
}

void RegisterColorizer::handle_scope_pop_command(ScopePopCommand* command) {
    pop_scopes(command->parameters[0].imm);
}

void RegisterColorizer::pop_command_register_usages(AsmCommand* command, int current_scope) {
    for (auto read_register : command->read_registers) {
        pop_nearest_register_usage(read_register, current_scope);
    }
    for (auto write_register : command->write_registers) {
        pop_nearest_register_usage(write_register, current_scope);
    }
}

void RegisterColorizer::destroy_command_register_usages(AsmCommand* command) {
    for (auto read_register : command->read_registers) {
        destroy_register_if_unused(read_register);
    }
    for (auto write_register : command->write_registers) {
        destroy_register_if_unused(write_register);
    }
}

void RegisterColorizer::handle_scope_command(ScopeCommand* command) {
    COLORIZER_DEBUG("; compiling <scope>\n");

    for (auto read_register : command->read_registers) {
        give_location_for_register(read_register);
    }
    for (auto write_register : command->write_registers) {
        give_location_for_register(write_register);
    }

    int current_scope = state_stack.size() - 1;

    push_state(((ScopeCommand*)command)->commands);

    pop_command_register_usages(command, current_scope);

    run_colorizer();
    pop_state();

    destroy_command_register_usages(command);

    flush_unused_registers();

#ifdef COLORIZER_DEBUG_ON
    COLORIZER_DEBUG("; free register stack: ");
    for (int j = 0; j < unused_registers_stack.size; j++) {
        COLORIZER_DEBUG("%s ", ASM_REGISTERS_64[unused_registers_stack.get(j)]);
    }
    COLORIZER_DEBUG("\n");
#endif
}

void RegisterColorizer::handle_general_command(AsmCommand* command) {
#ifdef COLORIZER_DEBUG_ON
    COLORIZER_DEBUG("; next instruction: ");
    command->dump(stdout, 0);
    COLORIZER_DEBUG("\n");
    COLORIZER_DEBUG("; input registers:\n");
#endif

    auto new_command = command->clone();
    if (command->type == COMMAND_JMP_LABEL) {
        label_map.insert({(JmpLabel*)command, (JmpLabel*)new_command});
    }
    if (command->type == COMMAND_CALL) {
        upload_symbol_registers();
    }

    for (auto read_register : command->read_registers) {
        handle_register_use(command, read_register, false);
    }

    colorize_command(new_command, false);
    flush_unused_registers();

    COLORIZER_DEBUG("; output registers\n");

    for (auto write_register : command->write_registers) {
        handle_register_use(command, write_register, true);
    }

    colorize_command(new_command, true);
    flush_unused_registers();
#ifdef COLORIZER_DEBUG_ON
    new_command->dump(stdout, 0);
    COLORIZER_DEBUG("\n");
    printf("; free register stack: ");
    for (int j = 0; j < unused_registers_stack.size; j++) {
        printf("%s ", ASM_REGISTERS_64[unused_registers_stack.get(j)]);
    }
    printf("\n");
    printf("; register map: ");
    for (int j = 0; j < 16; j++) {
        printf("%s=", ASM_REGISTERS_64[j]);
        if (machine_register_map[j] == -1) {
            printf("unused ");
        } else {
            printf("_r%lld ", machine_register_map[j]);
        }
    }
    printf("\n\n");
#endif

    output->root_list->commands.push_back(new_command);
    registers_released_by_command.clear();
}

void RegisterColorizer::handle_locate_reg_reg_command(LocateRegCommand* command) {
    auto parameter = command->parameters[0].reg;

    auto location =
        state->source->parent_buffer->descriptors.get_descriptor(command->get_register_location())
            ->last_register_location;

    locate_register_at_machine_register(parameter, location, command);
    pop_command_register_usages(command);
    destroy_command_register_usages(command);
    flush_unused_registers();
    registers_released_by_command.clear();
}

void RegisterColorizer::handle_locate_reg_stack_command(LocateRegCommand* command) {
    auto parameter = command->parameters[0].reg;
    locate_register_at_stack(parameter, command->get_stack_location());
    pop_command_register_usages(command);
    destroy_command_register_usages(command);
    flush_unused_registers();
    registers_released_by_command.clear();
}

void RegisterColorizer::locate_register_at_stack(AbstractRegister reg, int frame_position) {
    // This method is only used when user wants to
    // read function argument from stack. So,
    // to simplify things, we assume that this
    // function is only called for locating
    // function arguments.
    assert(frame_position <= -8);
    auto* descriptor = source->descriptors.get_descriptor(reg);

    assert(!descriptor->located_in_register && !descriptor->located_in_memory);

    descriptor->located_in_memory = true;
    descriptor->last_memory_position = frame_position;
}

void RegisterColorizer::locate_register_at_machine_register(AbstractRegister reg,
                                                            MachineRegister loc,
                                                             AsmCommand* command) {
    AbstractRegister used_register = machine_register_map[loc];

    if (used_register != -1) {
        COLORIZER_DEBUG(";  it would use register used by _r%llu\n", used_register);
        move_register(used_register, command);
        assert(gather_unused_register(loc));
    } else {
        if (!gather_unused_register(loc)) {
            COLORIZER_DEBUG(";  requested register may not be used for general purposes\n");
            destroy_register_if_unused(reg);
            return;
        }
    }

    COLORIZER_DEBUG(";  _r%lld now owns register %s\n", reg, ASM_REGISTERS_64[loc]);
    auto* descriptor = source->descriptors.get_descriptor(reg);

    descriptor->located_in_register = true;
    descriptor->last_register_location = loc;
    machine_register_map[loc] = reg;
}

void RegisterColorizer::run_colorizer() {
    auto list = state->source;
    bool end = false;
    for (auto i = list->commands.begin(); i != list->commands.end() && !end; ++i) {
        AsmCommand* command = *i;

        switch (command->type) {
        case COMMAND_COLORIZER_SCOPE_DEAD_END:
            end = true;
            break;
        case COMMAND_COLORIZER_REPEAT_SCOPE:
            break;
        case COMMAND_COLORIZER_REG_PRESERVE:
            handle_reg_preserve_command((RegPreserveCommand*)command);
            break;
        case COMMAND_COLORIZER_SCOPE_POP:
            handle_scope_pop_command((ScopePopCommand*)command);
            break;
        case COMMAND_COLORIZER_LOCATE_REG_REG:
            handle_locate_reg_reg_command((LocateRegCommand*)command);
            break;
        case COMMAND_COLORIZER_LOCATE_REG_STACK:
            handle_locate_reg_stack_command((LocateRegCommand*)command);
            break;
        case COMMAND_COLORIZER_SCOPE:
            handle_scope_command((ScopeCommand*)command);
            break;
        default:
            handle_general_command(command);
            break;
        }
    }

    pop_scopes(1);
}

void RegisterColorizer::replace_frame_commands() {
    if (frame_size % 16 != 0) {
        frame_size += 8;
    }

    auto list = output->root_list;
    int additional_rsp_offset = 0;
    std::vector<int> aligns = {};

    for (auto i = list->commands.begin(); i != list->commands.end();) {
        auto command = *i;

        if (command->type == COMMAND_PUSH)
            additional_rsp_offset++;
        else if (command->type == COMMAND_POP)
            additional_rsp_offset--;
        else if (command->type == COMMAND_COLORIZER_FRAME_CREATE) {
            if (frame_size == 0) {
                i = list->commands.erase(i);
                continue;
            }
            // push rbp
            // mov rbp, rsp
            // sub rsp, ...

            list->commands.insert(i, new PushCommand(output->descriptors.machine_register(rbp)));
            list->commands.insert(i, new MovCommand(CommandParameter::create_register_64(
                                                       output->descriptors.machine_register(rbp)),
                                                   CommandParameter::create_register_64(
                                                       output->descriptors.machine_register(rsp))));
            *i = new SubCommand(CommandParameter::create_register_64(
                                             output->descriptors.machine_register(rsp)),
                                         CommandParameter::create_imm32(frame_size));
        } else if (command->type == COMMAND_COLORIZER_FRAME_DESTROY) {
            if (frame_size == 0) {
                i = list->commands.erase(i);
                continue;
            }
            // add rsp, ...
            // pop rbp

            list->commands.insert(i, new AddCommand(CommandParameter::create_register_64(
                                                       output->descriptors.machine_register(rsp)),
                                                   CommandParameter::create_imm32(frame_size)));
            *i = new PopCommand(output->descriptors.machine_register(rbp));
        } else if (command->type == COMMAND_COLORIZER_ALIGN_STACK_BEFORE ||
                   command->type == COMMAND_COLORIZER_ALIGN_STACK_AFTER) {
            int stack_entries = ((AlignStackCommand*)command)->get_stack_entries();
            int total_bytes = abs(stack_entries) * 8;
            total_bytes += 8; // return address
            if (frame_size != 0) {
                total_bytes += 8; // frame pointer
                total_bytes += frame_size;
            }

            total_bytes += additional_rsp_offset;

            bool ignore = false;

            if (command->type == COMMAND_COLORIZER_ALIGN_STACK_BEFORE) {
                if (total_bytes % 16 != 0) {
                    additional_rsp_offset += 8;
                    *i = new SubCommand(CommandParameter::create_register_64(
                                                     output->descriptors.machine_register(rsp)),
                                                 CommandParameter::create_imm32(8));
                    aligns.push_back(8);
                } else {
                    ignore = true;
                    aligns.push_back(0);
                }
            } else {
                int align = aligns[aligns.size() - 1];
                aligns.pop_back();
                additional_rsp_offset -= align;
                align += stack_entries * 8;
                if (align != 0) {
                    *i = new AddCommand(CommandParameter::create_register_64(
                                                     output->descriptors.machine_register(rsp)),
                                                 CommandParameter::create_imm32(align));
                } else {
                    ignore = true;
                }
            }

            if (ignore) {
                i = list->commands.erase(i);
                continue;
            }
        } else {
            // [rbp + 8] => [rsp + 16]
            if (frame_size == 0)
                for (int j = 0; j < command->parameters.size(); j++) {
                    auto parameter = command->parameters[j];
                    if (parameter.type == PARAMETER_TYPE_MEMORY &&
                        parameter.memory.register_amount() == 1 &&
                        parameter.memory.displacement > 0) {
                        auto base =
                            output->descriptors.get_descriptor(parameter.memory.get_register(0));
                        if (base->has_register_constraint && base->last_register_location == rbp) {
                            parameter.memory.set_register(
                                0, output->descriptors.machine_register(rsp));
                            parameter.memory.displacement -= 8 - additional_rsp_offset;
                            command->parameters[j] = parameter;
                        }
                    }
                }
        }

        ++i;
    }
}

void RegisterColorizer::pop_nearest_register_usage(AbstractRegister reg, int max_scope) {
    if (max_scope == -1) {
        max_scope = state_stack.size() - 1;
    }
    for (int i = state_stack.size() - 1;; i--) {
        auto& state = state_stack[i];
        auto it = state.usage_heap_array.find(reg);
        if (it != state.usage_heap_array.end()) {
            if (i > max_scope)
                continue;
            auto heap_array = it->second;
            heap_array->pop();
            return;
        }
        assert(i > 0);
    }
}

void RegisterColorizer::destroy_register_if_unused(AbstractRegister reg) {
    auto descriptor = source->descriptors.get_descriptor(reg);
    if (descriptor->has_symbol_position)
        return;

    bool destroy = true;
    bool found = false;
    for (int i = state_stack.size() - 1; i >= 0; i--) {
        auto& state = state_stack[i];
        auto it = state.usage_heap_array.find(reg);

        if (it != state.usage_heap_array.end()) {
            auto heap_array = it->second;
            if (heap_array->length == 0) {
                delete heap_array;
                state.usage_heap_array.erase(reg);
                found = true;
            } else {
                destroy = false;
            }
        }
    }
    if (found && destroy) {
        release_register_location(reg, false);
    }
}

bin_heap<AbstractRegisterUsage>* RegisterColorizer::get_register_usage_heap(AbstractRegister reg) {
    for (int i = state_stack.size() - 1;; i--) {
        auto& state = state_stack[i];
        auto it = state.usage_heap_array.find(reg);
        if (it != state.usage_heap_array.end()) {
            return it->second;
        }
        if (i == 0)
            return nullptr;
    }
}

void RegisterColorizer::give_location_for_register(AbstractRegister reg) {
    assert(reg != -1);
    auto* descriptor = source->descriptors.get_descriptor(reg);

    if (descriptor->located_in_register || descriptor->located_in_memory ||
        descriptor->has_register_constraint || descriptor->has_symbol_position)
        return;

    MachineRegister effective_register = effective_unused_register(nullptr, false);
    if (effective_register == rinvalid) {
        descriptor->last_memory_position = frame_cell_for_register(reg);
        descriptor->located_in_memory = true;
    } else {
        descriptor->located_in_register = true;
        descriptor->last_register_location = effective_register;
        machine_register_map[effective_register] = reg;
    }
}

void RegisterColorizer::release_register_location(AbstractRegister reg, bool immediately) {
    assert(reg != -1);
    auto* descriptor = source->descriptors.get_descriptor(reg);

    if (descriptor->has_symbol_position) {
        move_register_to_symbol(reg);
    }
    if (descriptor->located_in_register) {
        descriptor->located_in_register = false;
        if (immediately) {
            release_register_immediately(descriptor->last_register_location, false);
        } else {
            release_register_after_command(descriptor->last_register_location);
        }

    } else if (descriptor->located_in_memory) {
        descriptor->located_in_memory = false;
        if (descriptor->owner == state->source) {
            release_frame_cell(descriptor->last_memory_position);
        }
    }
}

void RegisterColorizer::flush_unused_registers() {
    for (int j = 0; j < registers_to_release.size(); j++) {
        auto reg = registers_to_release[j];
        release_register_immediately(reg, true);
    }
    registers_to_release.clear();
}

void RegisterColorizer::release_register_immediately(MachineRegister reg,
                                                      bool released_by_command) {
    assert(reg != rinvalid);
    COLORIZER_DEBUG("; release immediately: %s\n", ASM_REGISTERS_64[reg]);

    machine_register_map[reg] = -1;
    unused_registers_stack.push_back(reg);
    if (released_by_command)
        registers_released_by_command.push_back(reg);
}

void RegisterColorizer::release_register_after_command(MachineRegister reg) {
    assert(reg != rinvalid);
    COLORIZER_DEBUG("; release after command: %s\n", ASM_REGISTERS_64[reg]);
    registers_to_release.push_back(reg);
}

bool RegisterColorizer::gather_unused_register(MachineRegister reg) {
    COLORIZER_DEBUG("; -> gather unused: %s\n", ASM_REGISTERS_64[reg]);

    for (int i = 0; i < unused_registers_stack.size(); i++) {
        if (unused_registers_stack[i] == reg) {
            unused_registers_stack.erase(unused_registers_stack.begin() + i);
            return true;
        }
    }

    return false;
}

MachineRegister RegisterColorizer::effective_unused_register(AsmCommand* cmd,
                                                                 bool ignore_used) {
    if (unused_registers_stack.size() > 0) {
        for (int i = unused_registers_stack.size() - 1; i >= 0; i--) {
            MachineRegister result = unused_registers_stack[i];
            if (!command_uses_register(cmd, result, ignore_used)) {
                unused_registers_stack.erase(unused_registers_stack.begin() + i);
                return result;
            }
        }
        // COLORIZER_DEBUG("; effective unused register: %s\n", ASM_REGISTERS_64[result]);
    }

    return rinvalid;
}

bool RegisterColorizer::is_unused_register(MachineRegister reg) {
    for (int i = 0; i < unused_registers_stack.size(); i++) {
        if (unused_registers_stack[i] == reg)
            return true;
    }
    return false;
}

void RegisterColorizer::transform_labels() {
    auto list = output->root_list;
    for (auto i = list->commands.begin(); i != list->commands.end(); ++i) {
        auto command = *i;

        for (int j = 0; j < command->parameters.size(); j++) {
            auto parameter = command->parameters[j];

            if (parameter.type == PARAMETER_TYPE_LABEL) {
                auto it = label_map.find(parameter.label);
                if (it != label_map.end()) {
                    parameter.label = it->second;
                } else {
                    parameter.label = nullptr;
                }
                command->parameters[j] = parameter;
            }
        }
    }
}

bool bonk::x86_backend::RegisterColorizer::command_uses_register(AsmCommand* cmd,
                                                                 MachineRegister reg,
                                                                  bool ignore_used) {
    if (cmd == nullptr)
        return false;
    for (auto i : cmd->read_registers) {
        auto descriptor = source->descriptors.get_descriptor(i);
        if ((descriptor->has_register_constraint || descriptor->located_in_register) &&
            descriptor->last_register_location == reg)
            return true;
        if (ignore_used)
            for (int j = 0; j < registers_released_by_command.size(); j++) {
                if (registers_released_by_command[j] == reg)
                    return true;
            }
    }

    for (auto i : cmd->write_registers) {
        auto descriptor = source->descriptors.get_descriptor(i);
        if ((descriptor->has_register_constraint || descriptor->located_in_register) &&
            descriptor->last_register_location == reg)
            return true;
        if (ignore_used)
            for (int j = 0; j < registers_released_by_command.size(); j++) {
                if (registers_released_by_command[j] == reg)
                    return true;
            }
    }

    return false;
}

void RegisterColorizer::move_register_to_symbol(AbstractRegister reg) {
    save_register_location(reg);

    auto* descriptor = source->descriptors.get_descriptor(reg);

    assert(descriptor->has_symbol_position);
    assert(reg != -1);
    assert(descriptor->located_in_register);

    output->root_list->commands.push_back(
        new MovCommand(CommandParameter::create_symbol(CommandParameterSymbol(true, descriptor->symbol_position)),
                        CommandParameter::create_register_64(output->descriptors.machine_register(
                            descriptor->last_register_location))));

    release_register_immediately(descriptor->last_register_location, false);

    descriptor->located_in_symbol = true;
    descriptor->located_in_register = false;
}

void RegisterColorizer::restore_register_from_symbol(AbstractRegister reg, MachineRegister target) {
    save_register_location(reg);

    auto* descriptor = source->descriptors.get_descriptor(reg);

    assert(descriptor->has_symbol_position);
    assert(reg != -1 && target != rinvalid);
    assert(descriptor->located_in_symbol && !descriptor->located_in_register);

    output->root_list->commands.push_back(new MovCommand(
        CommandParameter::create_register_64(output->descriptors.machine_register(target)),
        CommandParameter::create_symbol(
            CommandParameterSymbol(true, descriptor->symbol_position))));
    if (descriptor->owner == state->source) {
        release_frame_cell(descriptor->last_memory_position);
    }

    descriptor->located_in_register = true;
    descriptor->located_in_symbol = false;
    descriptor->last_register_location = target;

    machine_register_map[descriptor->last_register_location] = reg;
}

void RegisterColorizer::locate_register_at_symbol(AbstractRegister reg) {
    auto* descriptor = source->descriptors.get_descriptor(reg);

    descriptor->located_in_symbol = true;
}

void RegisterColorizer::upload_symbol_registers() {
    for (int i = 0; i < 16; i++) {
        auto reg = machine_register_map[i];
        if (reg == -1)
            continue;
        auto* descriptor = source->descriptors.get_descriptor(reg);
        if (descriptor->has_symbol_position) {
            move_register_to_symbol(reg);
        }
    }
}

} // namespace bonk::x86_backend