#pragma once

namespace bonk {

struct TreeNodeCallParameter;

}

#include "../../../utils/JsonSerializer.hpp"
#include "identifier.hpp"
#include "node.hpp"

namespace bonk {

struct TreeNodeCallParameter : TreeNode {

    TreeNodeIdentifier* parameter_name;
    TreeNode* parameter_value;

    TreeNodeCallParameter(TreeNodeIdentifier* name, TreeNode* value);

    ~TreeNodeCallParameter();

    void serialize(JsonSerializer* serializer);
};

} // namespace bonk
