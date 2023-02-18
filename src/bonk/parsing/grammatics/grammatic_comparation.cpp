/*
 * Comparation grammatic:
 * $MATH_FACTOR := $MATH_EXPRESSION ($COMPARATION_OPERATOR $MATH_EXPRESSION)*
 */

#include "grammatic_comparation.hpp"

namespace bonk {

tree_node* parse_grammatic_comparation(parser* parser) {

    tree_node* expression = parse_grammatic_math_expression(parser);
    if (!expression)
        return nullptr;

    tree_node* result = expression;

    do {
        lexeme* next = parser->next_lexeme();
        if (next->type != BONK_LEXEME_OPERATOR)
            break;
        operator_type operator_type = next->operator_data.operator_type;
        if (!is_comparation_operator(operator_type))
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

bool is_comparation_operator(operator_type oper) {
    switch (oper) {
    case BONK_OPERATOR_EQUALS:
    case BONK_OPERATOR_LESS_THAN:
    case BONK_OPERATOR_GREATER_THAN:
    case BONK_OPERATOR_LESS_OR_EQUAL_THAN:
    case BONK_OPERATOR_GREATER_OR_EQUAL_THAN:
    case BONK_OPERATOR_NOT_EQUAL:
        return true;
    default:
        return false;
    }
}

} // namespace bonk