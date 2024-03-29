
#pragma once

#include "bonk/frontend/frontend.hpp"

namespace bonk {

// This class is used to generate constructors and destructors for hives.
// It works on the AST level.

class HiveConstructorDestructorEarlyGenerator {
    FrontEnd& front_end;
    bonk::AST* current_ast = nullptr;

  public:
    explicit
        HiveConstructorDestructorEarlyGenerator(FrontEnd& front_end): front_end(front_end) {

    }

    bool generate(bonk::AST& ast);


  private:
    std::vector<bonk::TreeNodeVariableDefinition*> hive_fields;
    std::unique_ptr<TreeNode> generate_hive_constructor(TreeNodeHiveDefinition* hive_definition);
    std::unique_ptr<TreeNode> generate_hive_destructor(TreeNodeHiveDefinition* hive_definition);
};

} // namespace bonk