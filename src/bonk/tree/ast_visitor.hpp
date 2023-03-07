#pragma once

#include "ast.hpp"

namespace bonk {

class ASTVisitor {
  public:
    virtual void visit(TreeNodeProgram* node) = 0;
    virtual void visit(TreeNodeHelp* node) = 0;
    virtual void visit(TreeNodeIdentifier* node) = 0;
    virtual void visit(TreeNodeBlockDefinition* node) = 0;
    virtual void visit(TreeNodeVariableDefinition* node) = 0;
    virtual void visit(TreeNodeParameterListDefinition* node) = 0;
    virtual void visit(TreeNodeParameterList* node) = 0;
    virtual void visit(TreeNodeParameterListItem* node) = 0;
    virtual void visit(TreeNodeCodeBlock* node) = 0;
    virtual void visit(TreeNodeArrayConstant* node) = 0;
    virtual void visit(TreeNodeNumberConstant* node) = 0;
    virtual void visit(TreeNodeStringConstant* node) = 0;
    virtual void visit(TreeNodeBinaryOperation* node) = 0;
    virtual void visit(TreeNodeUnaryOperation* node) = 0;
    virtual void visit(TreeNodePrimitiveType* node) = 0;
    virtual void visit(TreeNodeManyType* node) = 0;
    virtual void visit(TreeNodeHiveAccess* node) = 0;
    virtual void visit(TreeNodeBonkStatement* node) = 0;
    virtual void visit(TreeNodeLoopStatement* node) = 0;
    virtual void visit(TreeNodeHiveDefinition* node) = 0;
    virtual void visit(TreeNodeCall* node) = 0;
};

} // namespace bonk