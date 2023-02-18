#pragma once

#include <cstdio>
#include <cstdlib>
#include <string>
#include "../../../utils/json_serializer.hpp"
#include "node.hpp"

namespace bonk {

struct tree_node_identifier : tree_node {
    std::string variable_name;

    tree_node_identifier(const std::string& name);

    bool contents_equal(tree_node_identifier* other);

    void print(FILE* file);

    void serialize(json_serializer* file) override;
};

} // namespace bonk
