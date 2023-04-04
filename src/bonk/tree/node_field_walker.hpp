#pragma once

#include <memory>

namespace bonk {

template <typename T> struct NodeFieldWalker {
    T& callback;

    explicit NodeFieldWalker(T& callback) : callback(callback) {
    }

    template <typename NodeType> void operator()(NodeType* node) {
        node->fields(callback);
    }
};

} // namespace bonk