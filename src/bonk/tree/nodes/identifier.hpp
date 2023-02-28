#pragma once

#include <cstdio>
#include <cstdlib>
#include <string>
#include "node.hpp"
#include "utils/json_serializer.hpp"

namespace bonk {

struct TreeNodeIdentifier : TreeNode {
    std::string variable_name;

    explicit TreeNodeIdentifier(const std::string& name);

    bool contents_equal(TreeNodeIdentifier* other);

    void print(FILE* file) const;

    void accept(ASTVisitor* visitor) override;
};

} // namespace bonk
