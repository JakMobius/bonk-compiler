/*
 * Comparation grammatic:
 * $MATH_FACTOR := $MATH_EXPRESSION ($COMPARATION_OPERATOR $MATH_EXPRESSION)*
 */

#include "grammatic_comparation.hpp"

namespace bonk {

TreeNode* parse_grammatic_comparation(Parser* parser) {

    TreeNode* expression = parse_grammatic_math_expression(parser);
    if (!expression)
        return nullptr;

    TreeNode* result = expression;

    do {
        Lexeme* next = parser->next_lexeme();
        if (next->type != BONK_LEXEME_OPERATOR)
            break;
        OperatorType operator_type = next->operator_data.operator_type;
        if (!is_comparation_operator(operator_type))
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

bool is_comparation_operator(OperatorType oper) {
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