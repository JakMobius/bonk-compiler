#pragma once

#include "hir.hpp"
#include "hir_printer.hpp"
#include "utils/streams.hpp"

namespace bonk {

class HIRGraphvizDumper {
  public:
    HIRGraphvizDumper(bonk::OutputStream& stream) : stream(stream) {
    }

    void dump(HIRProgram& program);
    void dump(HIRProcedure& procedure);
    void dump_instruction_text(bonk::HIRProgram& program, bonk::HIRInstruction* instruction);

  private:
    bonk::OutputStream& stream;
    std::stringstream buffer;
    bonk::StdOutputStream buffer_stream{buffer};
    bonk::HIRPrinter printer{buffer_stream};
    int procedure_base_index = 0;
};

} // namespace bonk