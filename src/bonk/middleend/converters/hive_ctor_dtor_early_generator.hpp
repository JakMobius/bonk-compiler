#pragma once

#include "bonk/middleend/middleend.hpp"

namespace bonk {

// This class is used to generate constructors and destructors for hives.
// It works on the AST level.

class HiveConstructorDestructorEarlyGenerator {
    MiddleEnd& middle_end;

  public:
    explicit HiveConstructorDestructorEarlyGenerator(MiddleEnd& middle_end): middle_end(middle_end) {

    }

    void generate(bonk::TreeNode* ast);


  private:
    std::vector<bonk::TreeNodeVariableDefinition*> hive_fields;
    std::unique_ptr<TreeNode> generate_hive_constructor(TreeNodeHiveDefinition* hive_definition);
    std::unique_ptr<TreeNode> generate_hive_destructor(TreeNodeHiveDefinition* hive_definition);
};

} // namespace bonk