#pragma once

#include "bonk/frontend/frontend.hpp"
#include "bonk/frontend/ast/ast_visitor.hpp"

namespace bonk {

class HiveConstructorCallReplacer : ASTVisitor {
    FrontEnd& front_end;
    AST* current_ast;

  public:
    explicit HiveConstructorCallReplacer(FrontEnd& front_end) : front_end(front_end) {
    }

    bool replace(AST& ast);

    void visit(TreeNodeHiveDefinition* node) override;
    void visit(TreeNodeIdentifier* node) override;
    void visit(TreeNodeVariableDefinition* node) override;
    void visit(TreeNodeBlockDefinition* node) override;
};

} // namespace bonk