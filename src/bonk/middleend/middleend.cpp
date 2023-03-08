
#include "middleend.hpp"
#include "type_annotating_visitor.hpp"

bool bonk::MiddleEnd::run_ast(TreeNode* ast) {
    bonk::TypeAnnotatingVisitor visitor {linked_compiler};

    ast->accept(&visitor);

    return true;
}
