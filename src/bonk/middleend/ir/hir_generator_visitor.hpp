#pragma once

#include <unordered_map>
#include "bonk/middleend/middleend.hpp"
#include "bonk/tree/ast_visitor.hpp"
#include "hir.hpp"
#include "ir.hpp"

namespace bonk {

struct HIRLoopContext {
    int loop_start_label;
    int loop_end_label;
};

class HIRGeneratorVisitor : ASTVisitor {

    MiddleEnd& middle_end;
    IRProgram* current_program;

    TreeNodeHiveDefinition* current_hive_definition = nullptr;
    TreeNodeBlockDefinition* current_block_definition = nullptr;
    IRProcedure* current_procedure = nullptr;
    IRBaseBlock* current_base_block = nullptr;

    std::optional<HIRLoopContext> current_loop_context {};

    std::vector<IRRegister> register_stack;

  public:
    HIRGeneratorVisitor(MiddleEnd& middle_end) : middle_end(middle_end) {
    }

    void generate(TreeNode* ast, IRProgram* program);

    HIROperationType convert_operation_to_hir(OperatorType type);
    HIRDataType convert_type_to_hir(Type* type);

    void visit(TreeNodeProgram* node) override;
    void visit(TreeNodeHelp* node) override;
    void visit(TreeNodeIdentifier* node) override;
    void visit(TreeNodeBlockDefinition* node) override;
    void visit(TreeNodeVariableDefinition* node) override;
    void visit(TreeNodeParameterList* node) override;
    void visit(TreeNodeParameterListItem* node) override;
    void visit(TreeNodeCodeBlock* node) override;
    void visit(TreeNodeArrayConstant* node) override;
    void visit(TreeNodeNumberConstant* node) override;
    void visit(TreeNodeStringConstant* node) override;
    void visit(TreeNodeBinaryOperation* node) override;
    void visit(TreeNodeUnaryOperation* node) override;
    void visit(TreeNodeHiveAccess* node) override;
    void visit(TreeNodeBonkStatement* node) override;
    void visit(TreeNodeLoopStatement* node) override;
    void visit(TreeNodeHiveDefinition* node) override;
    void visit(TreeNodeCall* node) override;
    void visit(TreeNodeBrekStatement* node) override;
};

} // namespace bonk