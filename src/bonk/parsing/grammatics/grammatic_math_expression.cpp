/*
 * Math expression grammatic:
 * $MATH_EXPRESSION := $MATH_TERM (['+'|'-']($MATH_TERM))*
 */

#include "grammatic_math_expression.hpp"

namespace bonk {

TreeNode* parse_grammatic_math_expression(Parser* parser) {
    unsigned long old_position = parser->position;
    TreeNode* term = parse_grammatic_math_term(parser);
    if (!term && old_position != parser->position)
        return nullptr;

    TreeNode* result = term;

    do {
        Lexeme* next = parser->next_lexeme();
        if (next->type != BONK_LEXEME_OPERATOR)
            break;
        OperatorType operator_type = next->operator_data.operator_type;
        if (operator_type != BONK_OPERATOR_PLUS && operator_type != BONK_OPERATOR_MINUS)
            break;

        parser->eat_lexeme();

        TreeNode* next_term = parse_grammatic_math_term(parser);
        if (!next_term) {
            if (!parser->linked_compiler->state) {
                parser->error("expected expression");
            }
            return nullptr;
        }

        auto* sum = new TreeNodeOperator(operator_type);
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