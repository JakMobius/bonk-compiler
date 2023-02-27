#pragma once

namespace bonk {

struct TreeNodeCheck;

}

#include "call_parameter.hpp"
#include "identifier.hpp"
#include "list.hpp"
#include "node.hpp"
#include "utils/json_serializer.hpp"

namespace bonk {

struct TreeNodeBlockDefinition : TreeNode {

    TreeNodeIdentifier* block_name;
    TreeNodeList<TreeNode*>* body;
    bool is_promise{};

    TreeNodeBlockDefinition();

    ~TreeNodeBlockDefinition() override;

    void serialize(JsonSerializer* file) override;
};

} // namespace bonk
