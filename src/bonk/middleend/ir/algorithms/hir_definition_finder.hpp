#pragma once

#include "bonk/middleend/ir/hir.hpp"
namespace bonk {

class HIRDefinitionFinder {
public:
    HIRDefinitionFinder() = default;

    std::unordered_map<IRRegister, HIRInstruction*> find_definitions(HIRProcedure& procedure);
    std::unordered_map<IRRegister, HIRInstruction*> find_typed_definitions(HIRProcedure& procedure, HIRInstructionType type);
};

}