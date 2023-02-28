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
    TreeNodeList* check_body;
    TreeNodeList* or_body;

    TreeNodeCheck();

    ~TreeNodeCheck() override;

    void accept(ASTVisitor* visitor) override;
};

} // namespace bonk
