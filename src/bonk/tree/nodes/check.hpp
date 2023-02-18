#pragma once
namespace bonk {

struct tree_node_check;

}

#include "../../../utils/json_serializer.hpp"
#include "list.hpp"
#include "node.hpp"

namespace bonk {

struct tree_node_check : tree_node {

    tree_node* condition;
    tree_node_list<tree_node*>* check_body;
    tree_node_list<tree_node*>* or_body;

    tree_node_check();

    ~tree_node_check() override;

    void serialize(json_serializer* serializer) override;
};

} // namespace bonk
