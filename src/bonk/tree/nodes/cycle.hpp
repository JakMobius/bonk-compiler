#pragma once
namespace bonk {

struct TreeNodeCycle;

}

#include "list.hpp"
#include "node.hpp"
#include "utils/json_serializer.hpp"

namespace bonk {

struct TreeNodeCycle : TreeNode {
    TreeNodeList* body = nullptr;

    TreeNodeCycle();

    void accept(ASTVisitor* visitor) override;
};

} // namespace bonk
