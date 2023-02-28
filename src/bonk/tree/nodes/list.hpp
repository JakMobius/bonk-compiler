#pragma once

#include <list>
#include "../../../utils/json_serializer.hpp"
#include "node.hpp"

namespace bonk {

struct TreeNodeList : TreeNode {

    std::list<TreeNode*> list;

    TreeNodeList();

    ~TreeNodeList() override;

    /* Override virtual methods */

    void accept(ASTVisitor* visitor) override;
};

} // namespace bonk
