#pragma once
namespace bonk {

struct tree_node_variable_definition;

}

#include "../../../utils/json_serializer.hpp"
#include "identifier.hpp"
#include "node.hpp"

namespace bonk {

struct tree_node_variable_definition : tree_node {
    bool is_contextual;
    tree_node_identifier* variable_name;
    tree_node* variable_value;

    tree_node_variable_definition(bool contextual, tree_node_identifier* identifier);

    ~tree_node_variable_definition();

    void serialize(json_serializer* file) override;
};

} // namespace bonk
