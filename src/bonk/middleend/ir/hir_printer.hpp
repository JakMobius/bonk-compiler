#pragma once

#include "bonk/frontend/frontend.hpp"
#include "hir.hpp"
#include "utils/streams.hpp"

namespace bonk {

class HIRPrinter {
    const OutputStream& stream;

  public:
    HIRPrinter(const OutputStream& stream) : stream(stream) {
    }

    void print(const bonk::HIRProgram& block) const;
    void print(const bonk::HIRProgram& program, const bonk::HIRProcedure& procedure) const;

    void print(const bonk::HIRProgram& program, const bonk::HIRLabelInstruction& instruction) const;
    void print(const bonk::HIRProgram& program, const bonk::HIRConstantLoadInstruction& instruction) const;
    void print(const bonk::HIRProgram& program, const bonk::HIRSymbolLoadInstruction& instruction) const;
    void print(const bonk::HIRProgram& program, const bonk::HIRJumpInstruction& instruction) const;
    void print(const bonk::HIRProgram& program, const bonk::HIRJumpNZInstruction& instruction) const;
    void print(const bonk::HIRProgram& program, const bonk::HIRCallInstruction& instruction) const;
    void print(const bonk::HIRProgram& program, const bonk::HIRReturnInstruction& instruction) const;
    void print(const bonk::HIRProgram& program, const bonk::HIROperationInstruction& instruction) const;
    void print(const bonk::HIRProgram& program, const bonk::HIRParameterInstruction& instruction) const;
    void print(const bonk::HIRProgram& program, const bonk::HIRMemoryLoadInstruction& instruction) const;
    void print(const bonk::HIRProgram& program, const bonk::HIRMemoryStoreInstruction& instruction) const;
    void print(const bonk::HIRProgram& program, const bonk::HIRIncRefCounterInstruction& instruction) const;
    void print(const bonk::HIRProgram& program, const bonk::HIRDecRefCounterInstruction& instruction) const;
    void print(const bonk::HIRProgram& program, const bonk::HIRFileInstruction& instruction) const;
    void print(const bonk::HIRProgram& program, const bonk::HIRLocationInstruction& instruction) const;

    void print(const bonk::HIRProgram& program, const bonk::HIRInstruction& instruction) const;
    void print_label(const bonk::HIRProgram& program, int label) const;

    void print(bonk::HIROperationType type) const;
    void print(HIRDataType type) const;

    void padding() const;
};

} // namespace bonk