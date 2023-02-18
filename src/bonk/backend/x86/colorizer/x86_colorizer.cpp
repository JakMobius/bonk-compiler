
#include "x86_colorizer.hpp"

namespace bonk::x86_backend {

unsigned long long colorizer_command_value(jmp_label* value) {
    return (unsigned long long)value;
}

bool colorizer_command_comparator(jmp_label* a, jmp_label* b) {
    return a == b;
}

const e_machine_register SYSTEM_V_OPTIMAL_REGISTERS[] = {
    // Callee-preserved, should be used last
    r15, r14, r13, r12, rbx,

    // Caller-preserved, should be used first
    r11, r10, r9, r8, rdi, rsi, rdx, rcx, rax};

register_colorizer::register_colorizer(command_buffer* input) {
    state = nullptr;
    source = input;

    for (auto i : SYSTEM_V_OPTIMAL_REGISTERS)
        unused_registers_stack.push_back(i);
    machine_register_map = (abstract_register*)(calloc(16, sizeof(abstract_register)));
    for (int i = 0; i < 16; i++)
        machine_register_map[i] = -1;
}

command_buffer* register_colorizer::colorize(command_buffer* commands) {
    register_colorizer ctx{commands};
    ctx.output = new command_buffer(&ctx.source->descriptors, ctx.source->file);

    ctx.push_state(commands->root_list);
    ctx.run_colorizer();
    ctx.replace_frame_commands();
    ctx.pop_state();

    ctx.transform_labels();

    return ctx.output;
}

register_colorizer::~register_colorizer() {
    frame_size = 0;
}

void register_colorizer::save_register_location(abstract_register reg) {
    assert(reg != -1);

    auto* descriptor = source->descriptors.get_descriptor(reg);

    // We should only save state of unowned registers
    if (descriptor->owner == state->source)
        return;

    if (state->relocation_info.find(reg) == state->relocation_info.end()) {
        state->relocation_info.insert({reg, *descriptor});
    }
}

void register_colorizer::move_register_to_register(abstract_register reg,
                                                   e_machine_register target) {
    assert(reg != -1 && target != rinvalid);
    save_register_location(reg);
    auto* descriptor = source->descriptors.get_descriptor(reg);

    // Register should have already been gathered, but should be unclaimed
    assert(!is_unused_register(target));
    assert(machine_register_map[target] == -1);

    output->root_list->insert_tail(new mov_command(
        command_parameter::create_register_64(output->descriptors.machine_register(target)),
        command_parameter::create_register_64(
            output->descriptors.machine_register(descriptor->last_register_location))));

    if (descriptor->located_in_register) {
        release_register_immediately(descriptor->last_register_location, false);
    }

    descriptor->located_in_register = true;
    descriptor->last_register_location = target;
    machine_register_map[descriptor->last_register_location] = reg;
}

void register_colorizer::move_register_to_stack(abstract_register reg, long long frame_position) {
    assert(reg != -1);
    save_register_location(reg);

    auto* descriptor = source->descriptors.get_descriptor(reg);

    assert(descriptor->located_in_register);

    output->root_list->insert_tail(
        new mov_command(command_parameter::create_memory(command_parameter_memory::create_reg_displ(
                            output->descriptors.machine_register(rbp), -frame_position)),
                        command_parameter::create_register_64(output->descriptors.machine_register(
                            descriptor->last_register_location))));

    release_register_immediately(descriptor->last_register_location, false);

    descriptor->last_memory_position = frame_position;
    descriptor->located_in_memory = true;
    descriptor->located_in_register = false;
}

void register_colorizer::restore_register_from_stack(abstract_register reg,
                                                     e_machine_register target) {
    assert(reg != -1 && target != rinvalid);
    save_register_location(reg);

    auto* descriptor = source->descriptors.get_descriptor(reg);
    assert(descriptor->located_in_memory && !descriptor->located_in_register);

    output->root_list->insert_tail(new mov_command(
        command_parameter::create_register_64(output->descriptors.machine_register(target)),
        command_parameter::create_memory(command_parameter_memory::create_reg_displ(
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

void register_colorizer::restore_register(abstract_register reg, e_machine_register target) {
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

int register_colorizer::frame_cell_for_register(abstract_register reg) {
    auto* descriptor = source->descriptors.get_descriptor(reg);

    auto it = state->relocation_info.find(reg);

    if (descriptor->owner != state->source && it != state->relocation_info.end()) {
        auto& old_descriptor = it->second;
        if (old_descriptor.located_in_memory)
            return old_descriptor.last_memory_position;
    }

    return retain_frame_cell();
}

void register_colorizer::move_register(abstract_register reg, asm_command* command) {
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
        e_machine_register register_to_move = effective_unused_register(command, true);

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

void register_colorizer::exchange_registers(abstract_register reg_a, abstract_register reg_b) {
    auto descriptor_a = source->descriptors.get_descriptor(reg_a);
    auto descriptor_b = source->descriptors.get_descriptor(reg_b);
    save_register_location(reg_a);
    save_register_location(reg_b);

    output->root_list->insert_tail(new xchg_command(
        command_parameter::create_register_64(
            output->descriptors.machine_register(descriptor_a->last_register_location)),
        command_parameter::create_register_64(
            output->descriptors.machine_register(descriptor_b->last_register_location))));

    e_machine_register temp = descriptor_a->last_register_location;
    descriptor_a->last_register_location = descriptor_b->last_register_location;
    descriptor_b->last_register_location = temp;

    machine_register_map[descriptor_a->last_register_location] =
        descriptor_a->last_register_location;
    machine_register_map[descriptor_b->last_register_location] =
        descriptor_b->last_register_location;
}

abstract_register register_colorizer::least_used_register_for_command(asm_command* command,
                                                                      bool is_write) {

    abstract_register result = -1;
    long long result_position = -1;
    long long result_scope = state_stack.size() - 1;

    for (int i = 0; i < 16; i++) {
        abstract_register placeholder = machine_register_map[i];

        // As unused registers are gathered in specific order,
        // with different algorithm, we should not check them
        // registers here
        if (placeholder == -1)
            continue;
        if (command_uses_register(command, e_machine_register(i), true))
            continue;

        bin_heap<abstract_register_usage>* usage_info = nullptr;
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

void register_colorizer::handle_register_use(asm_command* command, abstract_register reg,
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

            e_machine_register effective_register = effective_unused_register(command, false);

            if (effective_register == rinvalid) {
                abstract_register most_useless_register =
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

void register_colorizer::push_state(command_list* list) {
    register_colorize_context_state* old_state = nullptr;
    if (state_stack.size() > 0)
        old_state = &state_stack[state_stack.size() - 1];
    state_stack.push_back({list, old_state});
    state = &state_stack[state_stack.size() - 1];
}

void register_colorizer::pop_state() {

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

abstract_register register_colorizer::get_colorized_register(asm_command* command,
                                                             abstract_register reg, bool is_write) {
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

void register_colorizer::colorize_command(asm_command* command, bool is_write) {

    for (int i = 0; i < command->parameters.size(); i++) {
        command_parameter parameter = command->parameters[i];

        if (parameter.type == PARAMETER_TYPE_REG_64) {
            abstract_register reg = get_colorized_register(command, parameter.reg, is_write);

            command->parameters[i] = command_parameter::create_register_64(reg);
        } else if (parameter.type == PARAMETER_TYPE_REG_8) {
            abstract_register reg = get_colorized_register(command, parameter.reg, is_write);

            command->parameters[i] = command_parameter::create_register_8(reg);
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
            abstract_register reg = command->write_registers[i];
            auto last_location = source->descriptors.get_descriptor(reg)->last_register_location;
            command->write_registers[i] = source->descriptors.machine_register(last_location);
        }
    } else {
        for (int i = 0; i < command->read_registers.size(); i++) {
            abstract_register reg = command->read_registers[i];
            auto last_location = source->descriptors.get_descriptor(reg)->last_register_location;
            command->read_registers[i] = source->descriptors.machine_register(last_location);
        }
    }
}

long long register_colorizer::retain_frame_cell() {
    if (released_frame_cells.size()) {
        long long cell = released_frame_cells[released_frame_cells.size() - 1];
        released_frame_cells.pop_back();
        return cell;
    }
    frame_size += 8;
    return frame_size;
}

void register_colorizer::release_frame_cell(long long cell) {
    released_frame_cells.push_back(cell);
}

void register_colorizer::handle_reg_preserve_command(reg_preserve_command* command) {
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

void register_colorizer::pop_scopes(int scopes) {
    colorizer_state_pop_context restore_context = {this, scopes};
    restore_context.restore_registers();
}

void register_colorizer::handle_scope_pop_command(scope_pop_command* command) {
    pop_scopes(command->parameters[0].imm);
}

void register_colorizer::pop_command_register_usages(asm_command* command, int current_scope) {
    for (auto read_register : command->read_registers) {
        pop_nearest_register_usage(read_register, current_scope);
    }
    for (auto write_register : command->write_registers) {
        pop_nearest_register_usage(write_register, current_scope);
    }
}

void register_colorizer::destroy_command_register_usages(asm_command* command) {
    for (auto read_register : command->read_registers) {
        destroy_register_if_unused(read_register);
    }
    for (auto write_register : command->write_registers) {
        destroy_register_if_unused(write_register);
    }
}

void register_colorizer::handle_scope_command(scope_command* command) {
    COLORIZER_DEBUG("; compiling <scope>\n");

    for (auto read_register : command->read_registers) {
        give_location_for_register(read_register);
    }
    for (auto write_register : command->write_registers) {
        give_location_for_register(write_register);
    }

    int current_scope = state_stack.size() - 1;

    push_state(((scope_command*)command)->commands);

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

void register_colorizer::handle_general_command(asm_command* command) {
#ifdef COLORIZER_DEBUG_ON
    COLORIZER_DEBUG("; next instruction: ");
    command->dump(stdout, 0);
    COLORIZER_DEBUG("\n");
    COLORIZER_DEBUG("; input registers:\n");
#endif

    auto new_command = command->clone();
    if (command->type == COMMAND_JMP_LABEL) {
        label_map.insert({(jmp_label*)command, (jmp_label*)new_command});
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

    output->root_list->insert_tail(new_command);
    registers_released_by_command.clear();
}

void register_colorizer::handle_locate_reg_reg_command(locate_reg_command* command) {
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

void register_colorizer::handle_locate_reg_stack_command(locate_reg_command* command) {
    auto parameter = command->parameters[0].reg;
    locate_register_at_stack(parameter, command->get_stack_location());
    pop_command_register_usages(command);
    destroy_command_register_usages(command);
    flush_unused_registers();
    registers_released_by_command.clear();
}

void register_colorizer::locate_register_at_stack(abstract_register reg, int frame_position) {
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

void register_colorizer::locate_register_at_machine_register(abstract_register reg,
                                                             e_machine_register loc,
                                                             asm_command* command) {
    abstract_register used_register = machine_register_map[loc];

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

void register_colorizer::run_colorizer() {
    auto list = state->source;
    bool end = false;
    for (auto i = list->begin(); i != list->end() && !end; list->next_iterator(&i)) {
        asm_command* command = list->get(i);

        switch (command->type) {
        case COMMAND_COLORIZER_SCOPE_DEAD_END:
            end = true;
            break;
        case COMMAND_COLORIZER_REPEAT_SCOPE:
            break;
        case COMMAND_COLORIZER_REG_PRESERVE:
            handle_reg_preserve_command((reg_preserve_command*)command);
            break;
        case COMMAND_COLORIZER_SCOPE_POP:
            handle_scope_pop_command((scope_pop_command*)command);
            break;
        case COMMAND_COLORIZER_LOCATE_REG_REG:
            handle_locate_reg_reg_command((locate_reg_command*)command);
            break;
        case COMMAND_COLORIZER_LOCATE_REG_STACK:
            handle_locate_reg_stack_command((locate_reg_command*)command);
            break;
        case COMMAND_COLORIZER_SCOPE:
            handle_scope_command((scope_command*)command);
            break;
        default:
            handle_general_command(command);
            break;
        }
    }

    pop_scopes(1);
}

void register_colorizer::replace_frame_commands() {
    if (frame_size % 16 != 0) {
        frame_size += 8;
    }

    auto list = output->root_list;
    int additional_rsp_offset = 0;
    std::vector<int> aligns = {};

    for (auto i = list->begin(); i != list->end();) {
        auto command = list->get(i);

        if (command->type == COMMAND_PUSH)
            additional_rsp_offset++;
        else if (command->type == COMMAND_POP)
            additional_rsp_offset--;
        else if (command->type == COMMAND_COLORIZER_FRAME_CREATE) {
            if (frame_size == 0) {
                auto old = i;
                list->next_iterator(&i);
                list->remove(old);
                continue;
            }
            // push rbp
            // mov rbp, rsp
            // sub rsp, ...
            list->insert_before(i, new push_command(output->descriptors.machine_register(rbp)));
            list->insert_before(i, new mov_command(command_parameter::create_register_64(
                                                       output->descriptors.machine_register(rbp)),
                                                   command_parameter::create_register_64(
                                                       output->descriptors.machine_register(rsp))));
            list->set(i, new sub_command(command_parameter::create_register_64(
                                             output->descriptors.machine_register(rsp)),
                                         command_parameter::create_imm32(frame_size)));
        } else if (command->type == COMMAND_COLORIZER_FRAME_DESTROY) {
            if (frame_size == 0) {
                auto old = i;
                list->next_iterator(&i);
                list->remove(old);
                continue;
            }
            // add rsp, ...
            // pop rbp

            list->insert_before(i, new add_command(command_parameter::create_register_64(
                                                       output->descriptors.machine_register(rsp)),
                                                   command_parameter::create_imm32(frame_size)));
            list->set(i, new pop_command(output->descriptors.machine_register(rbp)));
        } else if (command->type == COMMAND_COLORIZER_ALIGN_STACK_BEFORE ||
                   command->type == COMMAND_COLORIZER_ALIGN_STACK_AFTER) {
            int stack_entries = ((align_stack_command*)command)->get_stack_entries();
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
                    list->set(i, new sub_command(command_parameter::create_register_64(
                                                     output->descriptors.machine_register(rsp)),
                                                 command_parameter::create_imm32(8)));
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
                    list->set(i, new add_command(command_parameter::create_register_64(
                                                     output->descriptors.machine_register(rsp)),
                                                 command_parameter::create_imm32(align)));
                } else {
                    ignore = true;
                }
            }

            if (ignore) {
                auto old = i;
                list->next_iterator(&i);
                list->remove(old);
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

        list->next_iterator(&i);
    }
}

void register_colorizer::pop_nearest_register_usage(abstract_register reg, int max_scope) {
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

void register_colorizer::destroy_register_if_unused(abstract_register reg) {
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

bin_heap<abstract_register_usage>*
register_colorizer::get_register_usage_heap(abstract_register reg) {
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

void register_colorizer::give_location_for_register(abstract_register reg) {
    assert(reg != -1);
    auto* descriptor = source->descriptors.get_descriptor(reg);

    if (descriptor->located_in_register || descriptor->located_in_memory ||
        descriptor->has_register_constraint || descriptor->has_symbol_position)
        return;

    e_machine_register effective_register = effective_unused_register(nullptr, false);
    if (effective_register == rinvalid) {
        descriptor->last_memory_position = frame_cell_for_register(reg);
        descriptor->located_in_memory = true;
    } else {
        descriptor->located_in_register = true;
        descriptor->last_register_location = effective_register;
        machine_register_map[effective_register] = reg;
    }
}

void register_colorizer::release_register_location(abstract_register reg, bool immediately) {
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

void register_colorizer::flush_unused_registers() {
    for (int j = 0; j < registers_to_release.size(); j++) {
        auto reg = registers_to_release[j];
        release_register_immediately(reg, true);
    }
    registers_to_release.clear();
}

void register_colorizer::release_register_immediately(e_machine_register reg,
                                                      bool released_by_command) {
    assert(reg != rinvalid);
    COLORIZER_DEBUG("; release immediately: %s\n", ASM_REGISTERS_64[reg]);

    machine_register_map[reg] = -1;
    unused_registers_stack.push_back(reg);
    if (released_by_command)
        registers_released_by_command.push_back(reg);
}

void register_colorizer::release_register_after_command(e_machine_register reg) {
    assert(reg != rinvalid);
    COLORIZER_DEBUG("; release after command: %s\n", ASM_REGISTERS_64[reg]);
    registers_to_release.push_back(reg);
}

bool register_colorizer::gather_unused_register(e_machine_register reg) {
    COLORIZER_DEBUG("; -> gather unused: %s\n", ASM_REGISTERS_64[reg]);

    for (int i = 0; i < unused_registers_stack.size(); i++) {
        if (unused_registers_stack[i] == reg) {
            unused_registers_stack.erase(unused_registers_stack.begin() + i);
            return true;
        }
    }

    return false;
}

e_machine_register register_colorizer::effective_unused_register(asm_command* cmd,
                                                                 bool ignore_used) {
    if (unused_registers_stack.size() > 0) {
        for (int i = unused_registers_stack.size() - 1; i >= 0; i--) {
            e_machine_register result = unused_registers_stack[i];
            if (!command_uses_register(cmd, result, ignore_used)) {
                unused_registers_stack.erase(unused_registers_stack.begin() + i);
                return result;
            }
        }
        // COLORIZER_DEBUG("; effective unused register: %s\n", ASM_REGISTERS_64[result]);
    }

    return rinvalid;
}

bool register_colorizer::is_unused_register(e_machine_register reg) {
    for (int i = 0; i < unused_registers_stack.size(); i++) {
        if (unused_registers_stack[i] == reg)
            return true;
    }
    return false;
}

void register_colorizer::transform_labels() {
    auto list = output->root_list;
    for (auto i = list->begin(); i != list->end(); list->next_iterator(&i)) {
        auto command = list->get(i);

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

bool bonk::x86_backend::register_colorizer::command_uses_register(asm_command* cmd,
                                                                  e_machine_register reg,
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

void register_colorizer::move_register_to_symbol(abstract_register reg) {
    save_register_location(reg);

    auto* descriptor = source->descriptors.get_descriptor(reg);

    assert(descriptor->has_symbol_position);
    assert(reg != -1);
    assert(descriptor->located_in_register);

    output->root_list->insert_tail(
        new mov_command(command_parameter::create_symbol(
                            command_parameter_symbol(true, descriptor->symbol_position)),
                        command_parameter::create_register_64(output->descriptors.machine_register(
                            descriptor->last_register_location))));

    release_register_immediately(descriptor->last_register_location, false);

    descriptor->located_in_symbol = true;
    descriptor->located_in_register = false;
}

void register_colorizer::restore_register_from_symbol(abstract_register reg,
                                                      e_machine_register target) {
    save_register_location(reg);

    auto* descriptor = source->descriptors.get_descriptor(reg);

    assert(descriptor->has_symbol_position);
    assert(reg != -1 && target != rinvalid);
    assert(descriptor->located_in_symbol && !descriptor->located_in_register);

    output->root_list->insert_tail(new mov_command(
        command_parameter::create_register_64(output->descriptors.machine_register(target)),
        command_parameter::create_symbol(
            command_parameter_symbol(true, descriptor->symbol_position))));
    if (descriptor->owner == state->source) {
        release_frame_cell(descriptor->last_memory_position);
    }

    descriptor->located_in_register = true;
    descriptor->located_in_symbol = false;
    descriptor->last_register_location = target;

    machine_register_map[descriptor->last_register_location] = reg;
}

void register_colorizer::locate_register_at_symbol(abstract_register reg) {
    auto* descriptor = source->descriptors.get_descriptor(reg);

    descriptor->located_in_symbol = true;
}

void register_colorizer::upload_symbol_registers() {
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