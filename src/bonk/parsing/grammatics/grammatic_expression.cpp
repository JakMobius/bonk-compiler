/*
 * Expression grammatic:
 * $EXPRESSION := $ASSIGNMENT | $LOGIC_EXPRESSION | $UNARY_OPERATOR
 */

#include "grammatic_expression.hpp"

namespace bonk {

tree_node* parse_grammatic_expression(parser* parser) {
    return parse_grammatic_expression_leveled(parser, false);
}

tree_node* parse_grammatic_expression_leveled(parser* parser, bool top_level) {

    tree_node* expression = parse_grammatic_logic_expression(parser);
    if (parser->linked_compiler->state)
        return nullptr;

    if (!expression) {
        expression = (tree_node*)parse_grammatic_unary_operator(parser);
        if (parser->linked_compiler->state)
            return nullptr;
    }

    if (top_level) {

        if (!expression) {
            expression = (tree_node*)parse_grammatic_var_definition(parser);
            if (parser->linked_compiler->state)
                return nullptr;
        }

        lexeme* next = parser->next_lexeme();
        if (next->type != BONK_LEXEME_SEMICOLON) {
            if (expression) {
                parser->error("expected semicolon");
            }
        } else {
            while (next->type == BONK_LEXEME_SEMICOLON) {
                parser->eat_lexeme();
                next = parser->next_lexeme();
            }
            if (!expression) {
                return parse_grammatic_expression(parser);
            }
        }
    }

    if (!expression) {
        expression = parse_grammatic_sub_block(parser);
        if (parser->linked_compiler->state)
            return nullptr;
    }

    return expression;
}

} // namespace bonk