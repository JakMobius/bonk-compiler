#pragma once

#include "ast.hpp"
#include "ast_visitor.hpp"
#include "utils/streams.hpp"

namespace bonk {

class ASTPrinter : public ASTVisitor {

    const OutputStream& stream;
    int depth = 0;

  public:
    explicit ASTPrinter(OutputStream& stream) : stream(stream) {
    }

    virtual void visit(TreeNodeHelp* node);
    virtual void visit(TreeNodeIdentifier* node);
    virtual void visit(TreeNodeBlockDefinition* node);
    virtual void visit(TreeNodeVariableDefinition* node);
    virtual void visit(TreeNodeParameterListDefinition* node);
    virtual void visit(TreeNodeParameterList* node);
    virtual void visit(TreeNodeParameterListItem* node);
    virtual void visit(TreeNodeCodeBlock* node);
    virtual void visit(TreeNodeArrayConstant* node);
    virtual void visit(TreeNodeNumberConstant* node);
    virtual void visit(TreeNodeStringConstant* node);
    virtual void visit(TreeNodeBinaryOperation* node);
    virtual void visit(TreeNodeUnaryOperation* node);
    virtual void visit(TreeNodePrimitiveType* node);
    virtual void visit(TreeNodeManyType* node);
    virtual void visit(TreeNodeHiveAccess* node);
    virtual void visit(TreeNodeBonkStatement* node);
    virtual void visit(TreeNodeBrekStatement* node);
    virtual void visit(TreeNodeLoopStatement* node);
    virtual void visit(TreeNodeHiveDefinition* node);
    virtual void visit(TreeNodeCall* node);

    void padding();

  private:
};

} // namespace bonk