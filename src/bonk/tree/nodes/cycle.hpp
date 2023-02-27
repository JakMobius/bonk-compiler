#pragma once
namespace bonk {

struct TreeNodeCycle;

}

#include "list.hpp"
#include "node.hpp"
#include "utils/json_serializer.hpp"

namespace bonk {

struct TreeNodeCycle : TreeNode {
    TreeNodeList<TreeNode*>* body = nullptr;

    TreeNodeCycle();

    void serialize(JsonSerializer* serializer) override;
};

} // namespace bonk
