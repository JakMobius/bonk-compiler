#pragma once

#include "bonk/backend/backend.hpp"
#include "bonk/middleend/ir/hir.hpp"

namespace bonk::qbe_backend {

class QBEBackend : public Backend {

    IRProgram* current_program = nullptr;
    std::vector<HIRProcedureParameter> call_parameters;

    bool compile_procedure(IRProcedure& procedure);
    bool compile_instruction(IRInstruction& instruction);
    bool compile_procedure_header(bonk::IRInstruction& instruction);
    void compile_procedure_footer();

    bool compile_instruction(HIRInstruction& instruction);
    bool compile_instruction(HIRLabel& instruction);
    bool compile_instruction(HIRConstantLoad& instruction);
    bool compile_instruction(HIRSymbolLoad& instruction);
    bool compile_instruction(HIROperation& instruction);
    bool compile_instruction(HIRJump& instruction);
    bool compile_instruction(HIRJumpNZ& instruction);
    bool compile_instruction(HIRCall& instruction);
    bool compile_instruction(HIRReturn& instruction);
    bool compile_instruction(HIRParameter& instruction);

    void print_hir_type(bonk::HIRDataType type);

    void padding();

    void print_hir_operation(HIROperationType type);

  public:
    bool compile_program(IRProgram& program) override;

    QBEBackend(Compiler& linked_compiler): Backend(linked_compiler) {};
};

}