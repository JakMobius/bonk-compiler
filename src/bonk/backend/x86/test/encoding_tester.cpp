
#include "encoding_tester.hpp"
#include "../command_dumper.hpp"
#include "../instructions/x86_add.hpp"
#include "../instructions/x86_conditional_instructions.hpp"
#include "../instructions/x86_idiv.hpp"
#include "../instructions/x86_imul.hpp"
#include "../instructions/x86_mov.hpp"
#include "../instructions/x86_sub.hpp"
#include "../instructions/x86_xor.hpp"

namespace bonk::x86_backend {

void test_encodings(FILE* target) {
    command_buffer* buffer = new command_buffer();

    buffer->root_list->insert_tail(new mov_command(command_parameter::create_register_64(rcx),
                                                   command_parameter::create_register_64(rdi)));

    //    buffer->root_list->insert_tail(and_command::create_reg8_reg8(rax, rbx,
    //    buffer->root_list)); buffer->root_list->insert_tail(or_command::create_reg8_reg8(rax, rbx,
    //    buffer->root_list));
    //
    //    buffer->root_list->insert_tail(mov_command::create_reg64_reg64(rax, rbx,
    //    buffer->root_list));
    //
    //    jmp_label* jmp_label = jmp_label::create_before(buffer->root_list);
    //    buffer->root_list->insert_tail(jmp_label);
    //
    //    buffer->root_list->insert_tail(jmp_command::create_before(jmp_label, buffer->root_list));
    //    buffer->root_list->insert_tail(je_command::create_before(jmp_label, buffer->root_list));
    //    buffer->root_list->insert_tail(jne_command::create_before(jmp_label, buffer->root_list));
    //    buffer->root_list->insert_tail(jg_command::create_before(jmp_label, buffer->root_list));
    //    buffer->root_list->insert_tail(jng_command::create_before(jmp_label, buffer->root_list));
    //    buffer->root_list->insert_tail(jl_command::create_before(jmp_label, buffer->root_list));
    //    buffer->root_list->insert_tail(jge_command::create_before(jmp_label, buffer->root_list));
    //
    //    buffer->root_list->insert_tail(sete_command::create_reg8(rax, buffer->root_list));
    //    buffer->root_list->insert_tail(setne_command::create_reg8(rax, buffer->root_list));
    //    buffer->root_list->insert_tail(setg_command::create_reg8(rax, buffer->root_list));
    //    buffer->root_list->insert_tail(setng_command::create_reg8(rax, buffer->root_list));
    //    buffer->root_list->insert_tail(setl_command::create_reg8(rax, buffer->root_list));
    //    buffer->root_list->insert_tail(setge_command::create_reg8(rax, buffer->root_list));

    // Check all mov reg, imm
    //    for(int i = 0; i < 16; i++) {
    //        buffer->root_list->insert_tail(mov_command::create_reg64_imm64(i, 0xBEBEBEBEFAFAFAFA,
    //        buffer->root_list));
    //    }

    // Check all mov reg, reg
    //    for(int i = 0; i < 16; i++) {
    //        for(int j = 0; j < 16; j++) {
    //            buffer->root_list->insert_tail(mov_command::create_reg64_reg64(i, j,
    //            buffer->root_list));
    //        }
    //    }

    // Check all mov reg, [reg]
    //    for(int i = 0; i < 16; i++) {
    //        for(int j = 0; j < 16; j++) {
    //            buffer->root_list->insert_tail(mov_command::create_reg64_mem64(
    //                i,
    //                command_parameter_memory::create_reg(j),
    //                buffer->root_list
    //            ));
    //        }
    //    }

    // Check all mov [reg], reg
    //    for(int i = 0; i < 16; i++) {
    //        for(int j = 0; j < 16; j++) {
    //            buffer->root_list->insert_tail(mov_command::create_mem64_reg64(
    //                command_parameter_memory::create_reg(i),
    //                j,
    //                buffer->root_list
    //            ));
    //        }
    //    }

    // Check all mov rax, [reg + reg]
    //    for(int i = 0; i < 16; i++) {
    //        for(int j = 0; j < 16; j++) {
    //            if(i == rsp && j == rsp) continue;
    //            buffer->root_list->insert_tail(mov_command::create_reg64_mem64(
    //                rax,
    //                command_parameter_memory::create_reg_reg(i, j),
    //                buffer->root_list
    //            ));
    //        }
    //    }

    // Check all mov rax, [reg + reg + disp32]
    //    for(int i = 0; i < 16; i++) {
    //        for(int j = 0; j < 16; j++) {
    //            if(i == rsp && j == rsp) continue;
    //            buffer->root_list->insert_tail(mov_command::create_reg64_mem64(
    //                rax,
    //                command_parameter_memory::create_reg_reg_displ(i, j, -512),
    //                buffer->root_list
    //            ));
    //        }
    //    }

    // Check all mov rax, [reg * scale + reg + disp32]
    //    for(int i = 0; i < 16; i++) {
    //        for(int j = 0; j < 16; j++) {
    //            if(i == rsp) continue;
    //            buffer->root_list->insert_tail(mov_command::create_reg64_mem64(
    //                rax,
    //                command_parameter_memory::create_reg_const_reg_displ(i, 2, j, -512),
    //                buffer->root_list
    //            ));
    //        }
    //    }

    // Check all mov rax, [reg + disp32]
    //    for(int i = 0; i < 16; i++) {
    //        buffer->root_list->insert_tail(mov_command::create_reg64_mem64(
    //            rax,
    //            command_parameter_memory::create_reg_displ(i, -512),
    //            buffer->root_list
    //        ));
    //    }

    // Check all mov rax, [reg * scale + disp32]
    //    for(int i = 0; i < 16; i++) {
    //        if(i == rsp) continue;
    //
    //        buffer->root_list->insert_tail(mov_command::create_reg64_mem64(
    //            rax,
    //            command_parameter_memory::create_reg_const_displ(i, 2, -512),
    //            buffer->root_list
    //        ));
    //    }

    //    buffer->root_list->insert_tail(xchg_command::create_reg64_reg64(rax, rbx,
    //    buffer->root_list)); buffer->root_list->insert_tail(xchg_command::create_reg64_mem64(rax,
    //    command_parameter_memory::create_reg(rbx), buffer->root_list));
    //    buffer->root_list->insert_tail(xchg_command::create_mem64_reg64(command_parameter_memory::create_reg(rbx),
    //    rax, buffer->root_list));

    command_dumper().dump_list(buffer->root_list, stdout, 0);

    command_encoder* bytes = buffer->to_bytes();

    fwrite(bytes->buffer.data(), 1, bytes->buffer.size(), target);

    delete bytes;
}

} // namespace bonk::x86_backend