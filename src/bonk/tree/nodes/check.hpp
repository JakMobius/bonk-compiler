#pragma once
namespace bonk {

struct TreeNodeCheck;

}

#include "list.hpp"
#include "node.hpp"
#include "utils/json_serializer.hpp"

namespace bonk {

struct TreeNodeCheck : TreeNode {

    TreeNode* condition;
    TreeNodeList<TreeNode*>* check_body;
    TreeNodeList<TreeNode*>* or_body;

    TreeNodeCheck();

    ~TreeNodeCheck() override;

    void serialize(JsonSerializer* serializer) override;
};

} // namespace bonk
