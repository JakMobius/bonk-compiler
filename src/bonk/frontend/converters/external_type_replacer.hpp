#pragma once

#include "bonk/frontend/frontend.hpp"
#include "bonk/frontend/ast/template_visitor.hpp"

namespace bonk {

class ExternalTypeReplacer : public TemplateVisitor<ExternalTypeReplacer> {
    FrontEnd& front_end;

  public:
    explicit ExternalTypeReplacer(FrontEnd& front_end)
        : TemplateVisitor<ExternalTypeReplacer>(*this), front_end(front_end) {
    }

    template <typename T> void operator()(T* node) {
        replace_types(node);
    }

    void replace_types(TreeNode* node);
    bool replace(bonk::AST& ast);
};

} // namespace bonk