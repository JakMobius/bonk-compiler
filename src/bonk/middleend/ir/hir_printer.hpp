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

    void print(const bonk::HIRProgram& program) const;
    void print(const bonk::HIRProcedure& procedure) const;

    void print(const bonk::HIRBaseBlock& block, const bonk::HIRLabelInstruction& instruction) const;
    void print(const bonk::HIRBaseBlock& block, const bonk::HIRConstantLoadInstruction& instruction) const;
    void print(const bonk::HIRBaseBlock& block, const bonk::HIRSymbolLoadInstruction& instruction) const;
    void print(const bonk::HIRBaseBlock& block, const bonk::HIRJumpInstruction& instruction) const;
    void print(const bonk::HIRBaseBlock& block, const bonk::HIRJumpNZInstruction& instruction) const;
    void print(const bonk::HIRBaseBlock& block, const bonk::HIRCallInstruction& instruction) const;
    void print(const bonk::HIRBaseBlock& block, const bonk::HIRReturnInstruction& instruction) const;
    void print(const bonk::HIRBaseBlock& block, const bonk::HIROperationInstruction& instruction) const;
    void print(const bonk::HIRBaseBlock& block, const bonk::HIRParameterInstruction& instruction) const;
    void print(const bonk::HIRBaseBlock& block, const bonk::HIRMemoryLoadInstruction& instruction) const;
    void print(const bonk::HIRBaseBlock& block, const bonk::HIRMemoryStoreInstruction& instruction) const;
    void print(const bonk::HIRBaseBlock& block, const bonk::HIRIncRefCounterInstruction& instruction) const;
    void print(const bonk::HIRBaseBlock& block, const bonk::HIRDecRefCounterInstruction& instruction) const;
    void print(const bonk::HIRBaseBlock& block, const bonk::HIRFileInstruction& instruction) const;
    void print(const bonk::HIRBaseBlock& block, const bonk::HIRLocationInstruction& instruction) const;
    void print(const bonk::HIRBaseBlock& block, const bonk::HIRPhiFunctionInstruction& instruction) const;

    void print(const bonk::HIRBaseBlock& block, const bonk::HIRInstruction& instruction) const;
    void print_label(const bonk::HIRBaseBlock& block, int label) const;

    void print(bonk::HIROperationType type) const;
    void print(HIRDataType type) const;

    void padding() const;
};

} // namespace bonk