#pragma once

#include "../../../utils/JsonSerializer.hpp"
#include "call_parameter.hpp"
#include "identifier.hpp"
#include "list.hpp"
#include "node.hpp"

namespace bonk {

struct TreeNodeCall : TreeNode {

    TreeNodeIdentifier* call_function;
    TreeNodeList<TreeNodeCallParameter*>* call_parameters;

    TreeNodeCall(TreeNodeIdentifier* function,
                   TreeNodeList<TreeNodeCallParameter*>* parameters);

    ~TreeNodeCall() override;

    void serialize(JsonSerializer* serializer) override;
};

} // namespace bonk
