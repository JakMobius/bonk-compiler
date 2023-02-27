/*
 * Assignment grammatic
 * $ASSIGNMENT := $IDENTIFIER '=' $EXPRESSION
 */

#include "grammatic_assignment.hpp"

namespace bonk {

TreeNode* parse_grammatic_assignment(Parser* parser) {

    Lexeme* lvalue = parser->next_lexeme();
    if (lvalue->type == BONK_LEXEME_NULL)
        return nullptr;

    parser->eat_lexeme();
    Lexeme* assignment = parser->next_lexeme();

    if (assignment->type != BONK_LEXEME_OPERATOR ||
        assignment->operator_data.operator_type != BONK_OPERATOR_ASSIGNMENT) {
        // This is not an assignment. Exit silently.
        parser->spit_lexeme();
        return nullptr;
    }

    if (lvalue->type != BONK_LEXEME_IDENTIFIER) {
        parser->linked_compiler->error_positioned(lvalue->position, "expression is not assignable");
        return nullptr;
    }

    parser->eat_lexeme();

    TreeNode* rvalue = parse_grammatic_expression(parser);
    if (!rvalue) {
        if (parser->linked_compiler->state)
            return nullptr;
        parser->error("expected value");
        return nullptr;
    }

    auto* expression = new TreeNodeOperator(BONK_OPERATOR_ASSIGNMENT);

    expression->left = lvalue->identifier_data.identifier;
    expression->right = rvalue;

    return expression;
}

} // namespace bonk