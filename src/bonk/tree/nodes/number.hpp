#pragma once
namespace bonk {

struct TreeNodeNumber;

}

#include "node.hpp"
#include "utils/json_serializer.hpp"

namespace bonk {

struct TreeNodeNumber : TreeNode {
    long long integer_value;
    long double float_value;

    TreeNodeNumber(long double float_value, long long integer_value);

    void serialize(JsonSerializer* serializer) override;
};

} // namespace bonk
