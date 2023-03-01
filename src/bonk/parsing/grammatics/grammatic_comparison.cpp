/*
 * Comparison grammatic:
 * $MATH_FACTOR := $MATH_EXPRESSION ($COMPARISON_OPERATOR $MATH_EXPRESSION)*
 */

#include "../parser.hpp"

namespace bonk {

std::unique_ptr<TreeNode> Parser::parse_comparison() {

    auto expression = parse_math_expression();
    if (!expression)
        return nullptr;

    auto result = std::move(expression);

    do {
        Lexeme* next = next_lexeme();
        if (next->type != BONK_LEXEME_OPERATOR)
            break;
        OperatorType operator_type = next->operator_data.operator_type;
        if (!is_comparison_operator(operator_type))
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

bool Parser::is_comparison_operator(OperatorType oper) {
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