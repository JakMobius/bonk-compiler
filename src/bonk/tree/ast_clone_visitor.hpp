#pragma once

#include "ast_visitor.hpp"
namespace bonk {

class ASTCloneVisitor : ASTVisitor {
  public:
    template <typename T> std::unique_ptr<T> clone(T* node) {
        if(!node) return nullptr;
        result = nullptr;
        node->accept(this);
        assert(result != nullptr);
        return std::unique_ptr<T>(static_cast<T*>(result.release()));
    }

    bool copy_source_positions = false;

  private:
    std::unique_ptr<TreeNode> result;

    void visit(TreeNodeProgram* node) override;
    void visit(TreeNodeHelp* node) override;
    void visit(TreeNodeIdentifier* node) override;
    void visit(TreeNodeBlockDefinition* node) override;
    void visit(TreeNodeVariableDefinition* node) override;
    void visit(TreeNodeParameterListDefinition* node) override;
    void visit(TreeNodeParameterList* node) override;
    void visit(TreeNodeParameterListItem* node) override;
    void visit(TreeNodeCodeBlock* node) override;
    void visit(TreeNodeArrayConstant* node) override;
    void visit(TreeNodeNumberConstant* node) override;
    void visit(TreeNodeStringConstant* node) override;
    void visit(TreeNodeBinaryOperation* node) override;
    void visit(TreeNodeUnaryOperation* node) override;
    void visit(TreeNodePrimitiveType* node) override;
    void visit(TreeNodeManyType* node) override;
    void visit(TreeNodeHiveAccess* node) override;
    void visit(TreeNodeBonkStatement* node) override;
    void visit(TreeNodeBrekStatement* node) override;
    void visit(TreeNodeLoopStatement* node) override;
    void visit(TreeNodeHiveDefinition* node) override;
    void visit(TreeNodeCall* node) override;

    template <typename T> std::unique_ptr<T> shallow_copy(T* node) {
        auto copy = std::make_unique<T>();
        if (copy_source_positions) {
            copy->source_position = node->source_position;
        }
        return copy;
    }
};

} // namespace bonk