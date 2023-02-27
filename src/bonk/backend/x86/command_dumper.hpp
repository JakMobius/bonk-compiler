
#pragma once

#include <cstdio>
#include "instructions/x86_command.hpp"
#include "x86_abstract_register.hpp"

namespace bonk::x86_backend {

class CommandDumper {
  public:
    CommandList* command_list = nullptr;

    CommandDumper() = default;

    virtual ~CommandDumper() = default;

    void dump(AsmCommand* command, FILE* file, int depth);

    void dump_list(struct CommandList* list, FILE* file, int depth);

  private:
    void dump_scope_command(AsmCommand* command, FILE* file, int depth);

    void depth_padding(FILE* file, int depth, bool is_label);

    void dump_register(AsmCommand* command, FILE* file, AbstractRegister reg) const;

    void dump_param_register(AsmCommand* command, FILE* file, CommandParameter param) const;
};

} // namespace bonk::x86_backend
