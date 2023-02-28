#pragma once
namespace bonk {

struct TreeNodeVariableDefinition;

}

#include "identifier.hpp"
#include "node.hpp"
#include "utils/json_serializer.hpp"

namespace bonk {

struct TreeNodeVariableDefinition : TreeNode {
    bool is_contextual;
    TreeNodeIdentifier* variable_name;
    TreeNode* variable_value;

    TreeNodeVariableDefinition(bool contextual, TreeNodeIdentifier* identifier);

    ~TreeNodeVariableDefinition() override;

    void accept(ASTVisitor* visitor) override;
};

} // namespace bonk
