#pragma once

#include "call_parameter.hpp"
#include "identifier.hpp"
#include "list.hpp"
#include "node.hpp"
#include "utils/json_serializer.hpp"

namespace bonk {

struct TreeNodeCall : TreeNode {

    TreeNodeIdentifier* call_function;
    TreeNodeList* call_parameters;

    TreeNodeCall(TreeNodeIdentifier* function, TreeNodeList* parameters);

    ~TreeNodeCall() override;

    void accept(ASTVisitor* visitor) override;

    void serialize(JsonSerializer* serializer) override;
};

} // namespace bonk
