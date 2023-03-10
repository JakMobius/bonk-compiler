#pragma once

#include "bonk/middleend/middleend.hpp"
#include "hir.hpp"
#include "utils/streams.hpp"

namespace bonk {

class HIRPrinter {
    const OutputStream& stream;

  public:

    MiddleEnd* middle_end = nullptr;

    HIRPrinter(const OutputStream& stream) : stream(stream) {
    }

    void print(const bonk::IRProgram& block) const;

    void print(const bonk::HIRLabel& instruction) const;
    void print(const bonk::HIRConstantLoad& instruction) const;
    void print(const bonk::HIRSymbolLoad& instruction) const;
    void print(const bonk::HIRJump& instruction) const;
    void print(const bonk::HIRJumpNZ& instruction) const;
    void print(const bonk::HIRCall& instruction) const;
    void print(const bonk::HIRReturn& instruction) const;
    void print(const bonk::HIROperation& instruction) const;
    void print(const bonk::HIRProcedure& instruction) const;
    void print(const bonk::HIRParameter& instruction) const;
    void print(const bonk::HIRInstruction& instruction) const;

    void print(bonk::HIROperationType type) const;
    void print(HIRDataType type) const;
};

} // namespace bonk