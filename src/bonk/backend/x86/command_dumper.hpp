
#pragma once

#include <cstdio>
#include "instructions/x86_command.hpp"
#include "x86_abstract_register.hpp"

namespace bonk::x86_backend {

class command_dumper {
  public:
    command_list* command_list = nullptr;

    command_dumper() {
    }

    virtual ~command_dumper() {
    }

    void dump(asm_command* command, FILE* file, int depth);

    void dump_list(struct command_list* list, FILE* file, int depth);

  private:
    void dump_scope_command(asm_command* command, FILE* file, int depth);

    void depth_padding(FILE* file, int depth, bool is_label);

    void dump_register(asm_command* command, FILE* file, abstract_register reg);

    void dump_param_register(asm_command* command, FILE* file, command_parameter param);
};

} // namespace bonk::x86_backend
