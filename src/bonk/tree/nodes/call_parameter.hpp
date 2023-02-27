#pragma once

namespace bonk {

struct TreeNodeCallParameter;

}

#include "identifier.hpp"
#include "node.hpp"
#include "utils/json_serializer.hpp"

namespace bonk {

struct TreeNodeCallParameter : TreeNode {

    TreeNodeIdentifier* parameter_name;
    TreeNode* parameter_value;

    TreeNodeCallParameter(TreeNodeIdentifier* name, TreeNode* value);

    ~TreeNodeCallParameter();

    void serialize(JsonSerializer* serializer);
};

} // namespace bonk
