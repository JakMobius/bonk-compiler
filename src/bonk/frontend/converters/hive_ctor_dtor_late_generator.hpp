#pragma once

#include "bonk/frontend/frontend.hpp"
#include "bonk/frontend/ast/ast_visitor.hpp"

namespace bonk {

class HiveConstructorDestructorLateGenerator : public ASTVisitor {
    FrontEnd& front_end;
    bonk::AST* current_ast = nullptr;

  public:
    explicit HiveConstructorDestructorLateGenerator(FrontEnd& front_end)
        : front_end(front_end) {
    }

    bool generate(bonk::AST& ast);

  private:
    void visit(TreeNodeHiveDefinition* node) override;
    void visit(TreeNodeBlockDefinition* node) override;

    void fill_constructor(TreeNodeBlockDefinition* ctor, TreeNodeHiveDefinition* hive);
    void fill_destructor(TreeNodeBlockDefinition* dtor, TreeNodeHiveDefinition* hive);
};

} // namespace bonk