#pragma once

#include "bonk/backend/backend.hpp"
#include "bonk/middleend/ir/hir.hpp"

namespace bonk::qbe_backend {

class QBEBackend : public Backend {

    IRProgram* current_program = nullptr;
    std::vector<HIRProcedureParameter> call_parameters;

    void compile_procedure(IRProcedure& procedure);
    void compile_instruction(IRInstruction& instruction);
    bool compile_procedure_header(bonk::IRInstruction& instruction);
    void compile_procedure_footer();

    void compile_instruction(HIRInstruction& instruction);
    void compile_instruction(HIRLabel& instruction);
    void compile_instruction(HIRConstantLoad& instruction);
    void compile_instruction(HIRSymbolLoad& instruction);
    void compile_instruction(HIROperation& instruction);
    void compile_instruction(HIRJump& instruction);
    void compile_instruction(HIRJumpNZ& instruction);
    void compile_instruction(HIRCall& instruction);
    void compile_instruction(HIRReturn& instruction);
    void compile_instruction(HIRParameter& instruction);
    void compile_instruction(HIRMemoryLoad& instruction);
    void compile_instruction(HIRMemoryStore& instruction);

    void print_hir_type(bonk::HIRDataType type);

    void padding();

    void print_hir_operation(HIROperationType type);

  public:
    void compile_program(IRProgram& program) override;

    QBEBackend(Compiler& linked_compiler): Backend(linked_compiler) {};
};

}