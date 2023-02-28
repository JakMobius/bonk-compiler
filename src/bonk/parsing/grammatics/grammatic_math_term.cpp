/*
 * Math multiplication grammatic:
 * $MATH_TERM := $MATH_FACTOR (['*'|'/']($MATH_FACTOR))*
 */

#include "../parser.hpp"

namespace bonk {

TreeNode* Parser::parse_math_term() {
    TreeNode* term = parse_math_factor();
    TreeNode* result = term;

    do {
        Lexeme* next = next_lexeme();
        if (next->type != BONK_LEXEME_OPERATOR)
            break;
        OperatorType operator_type = next->operator_data.operator_type;
        if (operator_type != BONK_OPERATOR_MULTIPLY && operator_type != BONK_OPERATOR_DIVIDE)
            break;

        eat_lexeme();

        TreeNode* next_term = parse_math_factor();
        if (!next_term) {
            if (!linked_compiler->state) {
                error("expected expression");
            }
            return nullptr;
        }

        auto* sum = new TreeNodeOperator(operator_type);
        sum->left = result;
        sum->right = next_term;
        result = sum;

    } while (true);

    return result;
}

} // namespace bonk