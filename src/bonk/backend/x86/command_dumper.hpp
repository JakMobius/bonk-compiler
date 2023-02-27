
#pragma once

#include <cstdio>
#include "instructions/x86_command.hpp"
#include "x86_abstract_register.hpp"

namespace bonk::x86_backend {

class command_dumper {
  public:
    CommandList* command_list = nullptr;

    command_dumper() {
    }

    virtual ~command_dumper() {
    }

    void dump(AsmCommand* command, FILE* file, int depth);

    void dump_list(struct CommandList* list, FILE* file, int depth);

  private:
    void dump_scope_command(AsmCommand* command, FILE* file, int depth);

    void depth_padding(FILE* file, int depth, bool is_label);

    void dump_register(AsmCommand* command, FILE* file, AbstractRegister reg);

    void dump_param_register(AsmCommand* command, FILE* file, CommandParameter param);
};

} // namespace bonk::x86_backend
