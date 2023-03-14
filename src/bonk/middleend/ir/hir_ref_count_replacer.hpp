#pragma once

#include "bonk/middleend/middleend.hpp"
#include "bonk/tree/ast_visitor.hpp"
#include "hir.hpp"
#include "ir.hpp"

namespace bonk {

class HIRRefCountReplacer : ASTVisitor {

    MiddleEnd& middle_end;
    IRBaseBlock* current_base_block;
    std::list<IRInstruction*>::iterator current_instruction_iterator;

  public:
    HIRRefCountReplacer(MiddleEnd& middle_end) : middle_end(middle_end) {
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
    void replace_ref_counters(IRProgram& program);
    void replace_ref_counters(IRProcedure& procedure);
};

} // namespace bonk