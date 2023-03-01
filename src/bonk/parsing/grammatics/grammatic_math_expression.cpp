/*
 * Math expression grammatic:
 * $MATH_EXPRESSION := $MATH_TERM (['+'|'-']($MATH_TERM))*
 */

#include "../parser.hpp"

namespace bonk {

std::unique_ptr<TreeNode> Parser::parse_math_expression() {
    unsigned long old_position = position;
    auto term = parse_math_term();
    if (!term && old_position != position)
        return nullptr;

    auto result = std::move(term);

    do {
        Lexeme* next = next_lexeme();
        if (next->type != BONK_LEXEME_OPERATOR)
            break;
        OperatorType operator_type = next->operator_data.operator_type;
        if (operator_type != BONK_OPERATOR_PLUS && operator_type != BONK_OPERATOR_MINUS)
            break;

        eat_lexeme();

        auto next_term = parse_math_term();
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