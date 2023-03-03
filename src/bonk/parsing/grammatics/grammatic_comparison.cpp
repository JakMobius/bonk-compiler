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
        if (next->type != LexemeType::l_operator)
            break;
        OperatorType operator_type = std::get<OperatorLexeme>(next->data).type;
        if (!is_comparison_operator(operator_type))
            break;

        eat_lexeme();

        auto next_term = parse_math_term();
        if (!next_term) {
            if (!linked_compiler.state) {
                linked_compiler.error().at(next_lexeme()->start_position) << "expected expression";
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
    case OperatorType::o_equal:
    case OperatorType::o_less:
    case OperatorType::o_greater:
    case OperatorType::o_less_equal:
    case OperatorType::o_greater_equal:
    case OperatorType::o_not_equal:
        return true;
    default:
        return false;
    }
}

} // namespace bonk