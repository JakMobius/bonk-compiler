#pragma once

#include "node_field_walker.hpp"
#include "template_visitor.hpp"
namespace bonk {

template <typename T>
struct ASTFieldWalker : public bonk::TemplateVisitor<bonk::NodeFieldWalker<T>> {
    bonk::NodeFieldWalker<T> node_field_walker;

    explicit ASTFieldWalker(T& callback)
        : node_field_walker(callback), bonk::TemplateVisitor<bonk::NodeFieldWalker<T>>(node_field_walker) {
    }
};

} // namespace bonk