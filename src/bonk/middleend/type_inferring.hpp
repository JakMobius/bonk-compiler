#pragma once

#include "bonk/tree/ast_visitor.hpp"
#include "type_annotating_visitor.hpp"
namespace bonk {

class TypeInferringVisitor : public ASTVisitor {
    TypeAnnotatingVisitor& visitor;
    std::vector<Type*> type_stack;
    std::vector<TreeNodeBlockDefinition*> block_stack;

  public:
    TypeInferringVisitor(TypeAnnotatingVisitor& visitor) : visitor(visitor) {

    }

    template<typename T>
    T* annotate(TreeNode* node) {
        auto type = std::make_unique<T>();
        T* pure_type = type.get();
        save_type(std::move(type));
        write_to_cache(node, pure_type);
        return pure_type;
    }

    Type* annotate(TreeNode* node, Type* type) {
        if(!type) return type;
        write_to_cache(node, type);
        return type;
    }

    void write_to_cache(TreeNode* node, Type* type);
    void save_type(std::unique_ptr<Type> type);

    Type* infer_type(TreeNode* node);

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
    void visit(TreeNodeLoopStatement* node) override;
    void visit(TreeNodeHiveDefinition* node) override;
    void visit(TreeNodeCall* node) override;
};
}