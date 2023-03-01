/*
 * Math multiplication grammatic:
 * $MATH_TERM := $MATH_FACTOR (['*'|'/']($MATH_FACTOR))*
 */

#include "../parser.hpp"

namespace bonk {

std::unique_ptr<TreeNode> Parser::parse_math_term() {
    auto result = parse_math_factor();

    do {
        Lexeme* next = next_lexeme();
        if (next->type != BONK_LEXEME_OPERATOR)
            break;
        OperatorType operator_type = next->operator_data.operator_type;
        if (operator_type != BONK_OPERATOR_MULTIPLY && operator_type != BONK_OPERATOR_DIVIDE)
            break;

        eat_lexeme();

        auto next_term = parse_math_factor();
        if (!next_term) {
            if (!linked_compiler->state) {
                error("expected expression");
            }
            return nullptr;
        }

        auto sum = std::make_unique<TreeNodeOperator>();
        sum->oper_type = operator_type;
        sum->left = std::move(result);
        sum->right = std::move(next_term);
        result = std::move(sum);

    } while (true);

    return result;
}

} // namespace bonk