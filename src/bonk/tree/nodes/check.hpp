#pragma once
namespace bonk {

struct TreeNodeCheck;

}

#include "../../../utils/JsonSerializer.hpp"
#include "list.hpp"
#include "node.hpp"

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
