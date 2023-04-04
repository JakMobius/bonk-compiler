#pragma once

#include "ast_visitor.hpp"

namespace bonk {

template <typename T> class TemplateVisitor : public ASTVisitor {
    T& callback;

  public:
    TemplateVisitor(T& callback) : callback(callback) {
    }

    void visit(TreeNodeProgram* node) override {
        callback(node);
    }
    void visit(TreeNodeHelp* node) override {
        callback(node);
    }
    void visit(TreeNodeIdentifier* node) override {
        callback(node);
    }
    void visit(TreeNodeBlockDefinition* node) override {
        callback(node);
    }
    void visit(TreeNodeVariableDefinition* node) override {
        callback(node);
    }
    void visit(TreeNodeParameterListDefinition* node) override {
        callback(node);
    }
    void visit(TreeNodeParameterList* node) override {
        callback(node);
    }
    void visit(TreeNodeParameterListItem* node) override {
        callback(node);
    }
    void visit(TreeNodeCodeBlock* node) override {
        callback(node);
    }
    void visit(TreeNodeArrayConstant* node) override {
        callback(node);
    }
    void visit(TreeNodeNumberConstant* node) override {
        callback(node);
    }
    void visit(TreeNodeStringConstant* node) override {
        callback(node);
    }
    void visit(TreeNodeBinaryOperation* node) override {
        callback(node);
    }
    void visit(TreeNodeUnaryOperation* node) override {
        callback(node);
    }
    void visit(TreeNodePrimitiveType* node) override {
        callback(node);
    }
    void visit(TreeNodeManyType* node) override {
        callback(node);
    }
    void visit(TreeNodeHiveAccess* node) override {
        callback(node);
    }
    void visit(TreeNodeBonkStatement* node) override {
        callback(node);
    }
    void visit(TreeNodeBrekStatement* node) override {
        callback(node);
    }
    void visit(TreeNodeLoopStatement* node) override {
        callback(node);
    }
    void visit(TreeNodeHiveDefinition* node) override {
        callback(node);
    }
    void visit(TreeNodeCall* node) override {
        callback(node);
    }
    void visit(TreeNodeCast* node) override {
        callback(node);
    }
    void visit(TreeNodeNull* node) override {
        callback(node);
    }
};

} // namespace bonk