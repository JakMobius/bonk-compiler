#pragma once
namespace bonk {

struct tree_node_number;

}

#include "../../../utils/json_serializer.hpp"
#include "node.hpp"

namespace bonk {

struct tree_node_number : tree_node {
    long long integer_value;
    long double float_value;

    tree_node_number(long double float_value, long long integer_value);

    void serialize(json_serializer* serializer) override;
};

} // namespace bonk
