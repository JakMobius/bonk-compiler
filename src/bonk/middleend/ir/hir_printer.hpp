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

    void print(const bonk::IRProgram& block) const;
    void print(const bonk::IRProgram& program, const bonk::IRProcedure& procedure) const;

    void print(const bonk::IRProgram& program, const bonk::HIRLabel& instruction) const;
    void print(const bonk::IRProgram& program, const bonk::HIRConstantLoad& instruction) const;
    void print(const bonk::IRProgram& program, const bonk::HIRSymbolLoad& instruction) const;
    void print(const bonk::IRProgram& program, const bonk::HIRJump& instruction) const;
    void print(const bonk::IRProgram& program, const bonk::HIRJumpNZ& instruction) const;
    void print(const bonk::IRProgram& program, const bonk::HIRCall& instruction) const;
    void print(const bonk::IRProgram& program, const bonk::HIRReturn& instruction) const;
    void print(const bonk::IRProgram& program, const bonk::HIROperation& instruction) const;
    void print(const bonk::IRProgram& program, const bonk::HIRProcedure& instruction) const;
    void print(const bonk::IRProgram& program, const bonk::HIRParameter& instruction) const;
    void print(const bonk::IRProgram& program, const bonk::HIRMemoryLoad& instruction) const;
    void print(const bonk::IRProgram& program, const bonk::HIRMemoryStore& instruction) const;
    void print(const bonk::IRProgram& program, const bonk::HIRIncRefCounter& instruction) const;
    void print(const bonk::IRProgram& program, const bonk::HIRDecRefCounter& instruction) const;
    void print(const bonk::IRProgram& program, const bonk::HIRFile& instruction) const;
    void print(const bonk::IRProgram& program, const bonk::HIRLocation& instruction) const;

    void print(const bonk::IRProgram& program, const bonk::HIRInstruction& instruction) const;
    void print_label(const bonk::IRProgram& program, int label) const;

    void print(bonk::HIROperationType type) const;
    void print(HIRDataType type) const;

    void padding() const;
};

} // namespace bonk