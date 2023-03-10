#pragma once

#include "bonk/middleend/middleend.hpp"
#include "bonk/tree/ast_visitor.hpp"

namespace bonk {

class HiveConstructorDestructorLateGenerator : public ASTVisitor {
    MiddleEnd& middle_end;

  public:
    explicit HiveConstructorDestructorLateGenerator(MiddleEnd& middle_end)
        : middle_end(middle_end) {
    }

    void generate(bonk::TreeNode* ast);

  private:
    void visit(TreeNodeHiveDefinition* node) override;
    void visit(TreeNodeBlockDefinition* node) override;

    void fill_constructor(TreeNodeBlockDefinition* ctor, TreeNodeHiveDefinition* hive);
    void fill_destructor(TreeNodeBlockDefinition* dtor, TreeNodeHiveDefinition* hive);
};

} // namespace bonk