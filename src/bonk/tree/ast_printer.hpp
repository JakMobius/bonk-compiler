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

    virtual void visit(TreeNodeHelp* node) override;
    virtual void visit(TreeNodeIdentifier* node) override;
    virtual void visit(TreeNodeBlockDefinition* node) override;
    virtual void visit(TreeNodeVariableDefinition* node) override;
    virtual void visit(TreeNodeParameterListDefinition* node) override;
    virtual void visit(TreeNodeParameterList* node) override;
    virtual void visit(TreeNodeParameterListItem* node) override;
    virtual void visit(TreeNodeCodeBlock* node) override;
    virtual void visit(TreeNodeArrayConstant* node) override;
    virtual void visit(TreeNodeNumberConstant* node) override;
    virtual void visit(TreeNodeStringConstant* node) override;
    virtual void visit(TreeNodeBinaryOperation* node) override;
    virtual void visit(TreeNodeUnaryOperation* node) override;
    virtual void visit(TreeNodePrimitiveType* node) override;
    virtual void visit(TreeNodeManyType* node) override;
    virtual void visit(TreeNodeHiveAccess* node) override;
    virtual void visit(TreeNodeBonkStatement* node) override;
    virtual void visit(TreeNodeBrekStatement* node) override;
    virtual void visit(TreeNodeLoopStatement* node) override;
    virtual void visit(TreeNodeHiveDefinition* node) override;
    virtual void visit(TreeNodeCall* node) override;
    virtual void visit(TreeNodeCast* node) override;
    virtual void visit(TreeNodeNull* node) override;

    void padding();

  private:
};

} // namespace bonk