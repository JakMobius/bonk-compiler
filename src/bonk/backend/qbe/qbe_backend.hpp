#pragma once

#include "bonk/backend/backend.hpp"
#include "bonk/middleend/ir/hir.hpp"

namespace bonk::qbe_backend {

class QBEBackend : public Backend {

    HIRProgram* current_program = nullptr;
    std::vector<HIRProcedureParameter> call_parameters;
    const bonk::OutputStream* output_stream = nullptr;

    void compile_procedure(HIRProcedure& procedure);
    void compile_instruction(HIRInstruction& instruction);
    bool compile_procedure_header(bonk::HIRInstruction& instruction);
    void compile_procedure_footer();

    void compile_instruction(HIRLabelInstruction& instruction);
    void compile_instruction(HIRConstantLoadInstruction& instruction);
    void compile_instruction(HIRSymbolLoadInstruction& instruction);
    void compile_instruction(HIROperationInstruction& instruction);
    void compile_instruction(HIRJumpInstruction& instruction);
    void compile_instruction(HIRJumpNZInstruction& instruction);
    void compile_instruction(HIRCallInstruction& instruction);
    void compile_instruction(HIRReturnInstruction& instruction);
    void compile_instruction(HIRParameterInstruction& instruction);
    void compile_instruction(HIRMemoryLoadInstruction& instruction);
    void compile_instruction(HIRMemoryStoreInstruction& instruction);
    void compile_instruction(HIRFileInstruction& instruction);
    void compile_instruction(HIRLocationInstruction& instruction);

    char get_hir_type(bonk::HIRDataType type, bool base_type = true);
    void print_comparison(HIROperationType type, HIRDataType operand_type);

    void padding();

  public:
    void compile_program(HIRProgram& program, const bonk::OutputStream& output) override;

    QBEBackend(Compiler& linked_compiler): Backend(linked_compiler) {};

    bool generate_debug_symbols = false;
    HIRFileInstruction* find_procedure_file_instruction(HIRProcedure& procedure);
};

}