#pragma once

namespace bonk {

struct tree_node_call_parameter;

}

#include "../../../utils/json_serializer.hpp"
#include "identifier.hpp"
#include "node.hpp"

namespace bonk {

struct tree_node_call_parameter : tree_node {

    tree_node_identifier* parameter_name;
    tree_node* parameter_value;

    tree_node_call_parameter(tree_node_identifier* name, tree_node* value);

    ~tree_node_call_parameter();

    void serialize(json_serializer* serializer);
};

} // namespace bonk
