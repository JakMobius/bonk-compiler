#pragma once

#include "../../../utils/json_serializer.hpp"
#include "../../../utils/list.hpp"
#include "node.hpp"

namespace bonk {

template <typename T> struct tree_node_list : tree_node {

    mlist<T> list;

    tree_node_list();

    ~tree_node_list() override;

    /* Override virtual methods */

    void serialize(json_serializer* serializer) override;
};

template <typename T> tree_node_list<T>::tree_node_list() {
    type = TREE_NODE_TYPE_LIST;
}

template <typename T> tree_node_list<T>::~tree_node_list() {
    for (auto i = list.begin(); i != list.end(); list.next_iterator(&i)) {
        delete list.get(i);
    }
}

template <typename T> void tree_node_list<T>::serialize(json_serializer* serializer) {
    tree_node::serialize(serializer);

    serializer->block_string_field("type", "list");
    serializer->block_start_array("contents");

    for (auto i = list.begin(); i != list.end(); list.next_iterator(&i)) {
        auto* element = list.get(i);
        if (element) {
            serializer->array_add_block();
            element->serialize(serializer);
            serializer->close_block();
        } else
            serializer->array_add_string(nullptr);
    }

    serializer->close_array();
}

} // namespace bonk
