#pragma once

namespace bonk {

struct tree_node_check;

}

#include "../../../utils/json_serializer.hpp"
#include "../../../utils/list.hpp"
#include "call_parameter.hpp"
#include "identifier.hpp"
#include "list.hpp"
#include "node.hpp"

namespace bonk {

struct tree_node_block_definition : tree_node {

    tree_node_identifier* block_name;
    tree_node_list<tree_node*>* body;
    bool is_promise;

    tree_node_block_definition();

    ~tree_node_block_definition();

    void serialize(json_serializer* file) override;
};

} // namespace bonk
