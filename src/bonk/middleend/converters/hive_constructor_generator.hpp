#pragma once

#include "bonk/middleend/middleend.hpp"

namespace bonk {

// This class is used to generate constructors and destructors for hives.
// It works on the AST level.

class
    HiveConstructorGenerator {
    MiddleEnd& middle_end;

  public:
    explicit HiveConstructorGenerator(MiddleEnd& middle_end): middle_end(middle_end) {

    }

    void generate(bonk::TreeNode* ast);

    int get_hive_footprint();

  private:
    std::vector<bonk::TreeNodeVariableDefinition*> hive_fields;
    std::unique_ptr<TreeNode> generate_hive_constructor(TreeNodeHiveDefinition* hive_definition);
};

} // namespace bonk