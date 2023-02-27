#pragma once
namespace bonk {

struct TreeNodeCycle;

}

#include "../../../utils/JsonSerializer.hpp"
#include "list.hpp"
#include "node.hpp"

namespace bonk {

struct TreeNodeCycle : TreeNode {
    TreeNodeList<TreeNode*>* body = nullptr;

    TreeNodeCycle();

    void serialize(JsonSerializer* serializer) override;
};

} // namespace bonk
