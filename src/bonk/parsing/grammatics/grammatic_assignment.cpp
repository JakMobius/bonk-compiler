/*
 * Assignment grammatic
 * $ASSIGNMENT := $IDENTIFIER '=' $EXPRESSION
 */

#include "../parser.hpp"

namespace bonk {

TreeNode* Parser::parse_assignment() {

    Lexeme* lvalue = next_lexeme();
    if (lvalue->type == BONK_LEXEME_NULL)
        return nullptr;

    eat_lexeme();
    Lexeme* assignment = next_lexeme();

    if (assignment->type != BONK_LEXEME_OPERATOR ||
        assignment->operator_data.operator_type != BONK_OPERATOR_ASSIGNMENT) {
        // This is not an assignment. Exit silently.
        spit_lexeme();
        return nullptr;
    }

    if (lvalue->type != BONK_LEXEME_IDENTIFIER) {
        linked_compiler->error_positioned(lvalue->position, "expression is not assignable");
        return nullptr;
    }

    eat_lexeme();

    TreeNode* rvalue = parse_expression();
    if (!rvalue) {
        if (linked_compiler->state)
            return nullptr;
        error("expected value");
        return nullptr;
    }

    auto* expression = new TreeNodeOperator(BONK_OPERATOR_ASSIGNMENT);

    expression->left = lvalue->identifier_data.identifier;
    expression->right = rvalue;

    return expression;
}

} // namespace bonk