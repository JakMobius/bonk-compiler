#pragma once
namespace bonk {

struct tree_node_cycle;

}

#include "../../../utils/json_serializer.hpp"
#include "list.hpp"
#include "node.hpp"

namespace bonk {

struct tree_node_cycle : tree_node {
    tree_node_list<tree_node*>* body = nullptr;

    tree_node_cycle();

    void serialize(json_serializer* serializer) override;
};

} // namespace bonk
