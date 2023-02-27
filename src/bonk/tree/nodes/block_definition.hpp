#pragma once

namespace bonk {

struct TreeNodeCheck;

}

#include "../../../utils/JsonSerializer.hpp"
#include "../../../utils/list.hpp"
#include "call_parameter.hpp"
#include "identifier.hpp"
#include "list.hpp"
#include "node.hpp"

namespace bonk {

struct TreeNodeBlockDefinition : TreeNode {

    TreeNodeIdentifier* block_name;
    TreeNodeList<TreeNode*>* body;
    bool is_promise;

    TreeNodeBlockDefinition();

    ~TreeNodeBlockDefinition();

    void serialize(JsonSerializer* file) override;
};

} // namespace bonk
