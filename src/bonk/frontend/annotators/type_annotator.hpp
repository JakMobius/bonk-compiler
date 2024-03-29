#pragma once

namespace bonk {

class TypeAnnotator;
class FrontEnd;

}

#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include "bonk/frontend/ast/ast_visitor.hpp"
#include "types.hpp"

namespace bonk {

class TypeAnnotator : public ASTVisitor {

  public:
    explicit TypeAnnotator(FrontEnd& front_end);
    ~TypeAnnotator() = default;

    bool annotate_ast(AST& ast);

  private:
    bool errors_occurred = false;
    FrontEnd& front_end;

    void visit(TreeNodeHiveDefinition* node) override;
    void visit(TreeNodeVariableDefinition* node) override;
    void visit(TreeNodeBlockDefinition* node) override;
    void visit(TreeNodeHiveAccess* node) override;
    void visit(TreeNodeCall* node) override;
    void visit(TreeNodeCast* node) override;
    void visit(TreeNodeParameterListItem* node) override;

    void visit(TreeNodeArrayConstant* node) override;
    void visit(TreeNodeNumberConstant* node) override;
    void visit(TreeNodeStringConstant* node) override;
    void visit(TreeNodeBinaryOperation* node) override;
    void visit(TreeNodeUnaryOperation* node) override;
    void visit(TreeNodePrimitiveType* node) override;
    void visit(TreeNodeManyType* node) override;
    void visit(TreeNodeBonkStatement* node) override;
    void visit(TreeNodeNull* node) override;

    Type* infer_type(TreeNode* node);

};

} // namespace bonk