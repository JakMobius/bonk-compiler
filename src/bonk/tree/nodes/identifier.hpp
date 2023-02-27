#pragma once

#include <cstdio>
#include <cstdlib>
#include <string>
#include "../../../utils/JsonSerializer.hpp"
#include "node.hpp"

namespace bonk {

struct TreeNodeIdentifier : TreeNode {
    std::string variable_name;

    TreeNodeIdentifier(const std::string& name);

    bool contents_equal(TreeNodeIdentifier* other);

    void print(FILE* file);

    void serialize(JsonSerializer* file) override;
};

} // namespace bonk
