#pragma once

#include "bonk/middleend/middleend.hpp"
#include "bonk/tree/ast_visitor.hpp"

namespace bonk {

class HiveConstructorCallReplacer : ASTVisitor {
    MiddleEnd& middle_end;
    AST* current_ast;

  public:
    explicit HiveConstructorCallReplacer(MiddleEnd& middle_end) : middle_end(middle_end) {
    }

    bool replace(AST& ast);

    void visit(TreeNodeHiveDefinition* node) override;
    void visit(TreeNodeIdentifier* node) override;
    void visit(TreeNodeVariableDefinition* node) override;
    void visit(TreeNodeBlockDefinition* node) override;
};

} // namespace bonk