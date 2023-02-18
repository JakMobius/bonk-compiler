#pragma once

#include "../../../utils/json_serializer.hpp"
#include "call_parameter.hpp"
#include "identifier.hpp"
#include "list.hpp"
#include "node.hpp"

namespace bonk {

struct tree_node_call : tree_node {

    tree_node_identifier* call_function;
    tree_node_list<tree_node_call_parameter*>* call_parameters;

    tree_node_call(tree_node_identifier* function,
                   tree_node_list<tree_node_call_parameter*>* parameters);

    ~tree_node_call() override;

    void serialize(json_serializer* serializer) override;
};

} // namespace bonk
