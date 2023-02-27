#pragma once

#include "../../../utils/JsonSerializer.hpp"
#include "../../../utils/list.hpp"
#include "node.hpp"

namespace bonk {

template <typename T> struct TreeNodeList : TreeNode {

    MList<T> list;

    TreeNodeList();

    ~TreeNodeList() override;

    /* Override virtual methods */

    void serialize(JsonSerializer* serializer) override;
};

template <typename T> TreeNodeList<T>::TreeNodeList() {
    type = TREE_NODE_TYPE_LIST;
}

template <typename T> TreeNodeList<T>::~TreeNodeList() {
    for (auto i = list.begin(); i != list.end(); list.next_iterator(&i)) {
        delete list.get(i);
    }
}

template <typename T> void TreeNodeList<T>::serialize(JsonSerializer* serializer) {
    TreeNode::serialize(serializer);

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
