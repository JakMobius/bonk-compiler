#pragma once
namespace bonk {

struct TreeNodeNumber;

}

#include "../../../utils/JsonSerializer.hpp"
#include "node.hpp"

namespace bonk {

struct TreeNodeNumber : TreeNode {
    long long integer_value;
    long double float_value;

    TreeNodeNumber(long double float_value, long long integer_value);

    void serialize(JsonSerializer* serializer) override;
};

} // namespace bonk
