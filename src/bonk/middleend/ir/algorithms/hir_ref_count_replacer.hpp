#pragma once

#include "bonk/frontend/ast/ast_visitor.hpp"
#include "bonk/frontend/frontend.hpp"
#include "bonk/middleend/ir/hir.hpp"
#include "bonk/middleend/ir/instruction_pool.hpp"

namespace bonk {

class HIRRefCountReplacer : ASTVisitor {

    HIRProgram* current_program;
    HIRProcedure* current_procedure;
    HIRBaseBlock* current_base_block;
    std::list<HIRInstruction*>::iterator current_instruction_iterator;

  public:
    HIRRefCountReplacer() {
    }

    IRRegister get_reference_address(IRRegister hive_register);
    IRRegister adjust_reference_count(IRRegister reference_address, int64_t delta);
    bonk::IRRegister load_reference_count(bonk::IRRegister reference_address);
    void write_reference_count(IRRegister reference_address, IRRegister value);
    void increase_reference_count(IRRegister register_id);
    void decrease_reference_count(IRRegister register_id, TreeNodeHiveDefinition* hive_definition);
    void replace_ref_counters(HIRInstruction* instruction);
    void call_destructor(TreeNodeHiveDefinition* hive_definition, IRRegister register_id);

    template <typename T, typename... Args> T* add_instruction(Args&&... args) {
        auto instruction = current_base_block->instruction<T>(std::forward<Args>(args)...);
        current_base_block->instructions.insert(current_instruction_iterator, instruction);
        return instruction;
    }

    void remove_instruction();

  public:
    bool replace_ref_counters(HIRProgram& program);
    bool replace_ref_counters(HIRProcedure& procedure);
};

} // namespace bonk