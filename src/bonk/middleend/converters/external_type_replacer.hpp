#pragma once

#include "bonk/middleend/middleend.hpp"
#include "bonk/tree/template_visitor.hpp"

namespace bonk {

class ExternalTypeReplacer : public TemplateVisitor<ExternalTypeReplacer> {
    MiddleEnd& middle_end;

  public:
    explicit ExternalTypeReplacer(MiddleEnd& middle_end)
        : TemplateVisitor<ExternalTypeReplacer>(*this), middle_end(middle_end) {
    }

    template <typename T> void operator()(T* node) {
        replace_types(node);
    }

    void replace_types(TreeNode* node);
    void replace(bonk::AST& ast);
};

} // namespace bonk