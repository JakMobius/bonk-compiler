#pragma once
namespace bonk {

struct TreeNodeVariableDefinition;

}

#include "../../../utils/JsonSerializer.hpp"
#include "identifier.hpp"
#include "node.hpp"

namespace bonk {

struct TreeNodeVariableDefinition : TreeNode {
    bool is_contextual;
    TreeNodeIdentifier* variable_name;
    TreeNode* variable_value;

    TreeNodeVariableDefinition(bool contextual, TreeNodeIdentifier* identifier);

    ~TreeNodeVariableDefinition();

    void serialize(JsonSerializer* file) override;
};

} // namespace bonk
