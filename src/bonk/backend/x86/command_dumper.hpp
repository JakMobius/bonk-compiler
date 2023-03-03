
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

    void dump(AsmCommand* command, const OutputStream& output, int depth);

    void dump_list(struct CommandList* list, const OutputStream& output, int depth);

  private:
    void dump_scope_command(AsmCommand* command, const OutputStream& output, int depth);

    void depth_padding(const OutputStream& output, int depth, bool is_label);

    void dump_register(const OutputStream& output, AbstractRegister reg) const;

    void dump_param_register(AsmCommand* command, const OutputStream& output, CommandParameter param) const;
};

} // namespace bonk::x86_backend
