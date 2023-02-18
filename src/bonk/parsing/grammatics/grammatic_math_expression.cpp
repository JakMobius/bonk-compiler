/*
 * Math expression grammatic:
 * $MATH_EXPRESSION := $MATH_TERM (['+'|'-']($MATH_TERM))*
 */

#include "grammatic_math_expression.hpp"

namespace bonk {

tree_node* parse_grammatic_math_expression(parser* parser) {
    unsigned long old_position = parser->position;
    tree_node* term = parse_grammatic_math_term(parser);
    if (!term && old_position != parser->position)
        return nullptr;

    tree_node* result = term;

    do {
        lexeme* next = parser->next_lexeme();
        if (next->type != BONK_LEXEME_OPERATOR)
            break;
        operator_type operator_type = next->operator_data.operator_type;
        if (operator_type != BONK_OPERATOR_PLUS && operator_type != BONK_OPERATOR_MINUS)
            break;

        parser->eat_lexeme();

        tree_node* next_term = parse_grammatic_math_term(parser);
        if (!next_term) {
            if (!parser->linked_compiler->state) {
                parser->error("expected expression");
            }
            return nullptr;
        }

        auto* sum = new tree_node_operator(operator_type);
        if (!sum) {
            parser->linked_compiler->out_of_memory();
            return nullptr;
        }
        sum->left = result;
        sum->right = next_term;
        result = sum;

    } while (true);

    return result;
}

} // namespace bonk