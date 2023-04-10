#pragma once

#include <cassert>
#include "ast_field_walker.hpp"
#include "ast_visitor.hpp"

namespace bonk {

class ASTCloneVisitor : public TemplateVisitor<ASTCloneVisitor> {
  public:
    ASTCloneVisitor() : TemplateVisitor(*this) {
    }

    template <typename T> std::unique_ptr<T> clone(T* node) {
        if (!node)
            return nullptr;
        auto old_cloned_node = cloned_node;
        auto old_result = std::move(result);
        node->accept(this);
        assert(result != nullptr);
        T* result_ptr = static_cast<T*>(result.release());
        cloned_node = old_cloned_node;
        result = std::move(old_result);
        return std::unique_ptr<T>(result_ptr);
    }

    bool copy_source_positions = false;

    template <typename T> void clone(std::unique_ptr<T>& from, std::unique_ptr<T>& to) {
        if (from) {
            to = clone(from.get());
        }
    }

    void clone(bonk::ParserPosition& from, bonk::ParserPosition& to) {
        if (copy_source_positions) {
            to = from;
        }
    }

    template <typename T>
    void clone(std::list<std::unique_ptr<T>>& from, std::list<std::unique_ptr<T>>& to) {
        for (auto& element : from) {
            clone(element, to.emplace_back());
        }
    }

    template <typename T> void clone(T& from, T& to) {
        to = from;
    }

    template <typename T> void operator()(T& value, std::string_view name) {
        T& result_value = *(T*)((char*)result.get() + ((char*)(&value) - (char*)cloned_node));

        clone(value, result_value);
    }

    template <typename T> void operator()(T* node) {
        cloned_node = node;
        result = std::make_unique<T>();

        node->fields(*this);
    }

  protected:
    TreeNode* cloned_node = nullptr;
    std::unique_ptr<TreeNode> result;
};

} // namespace bonk