/*
 * Math factor grammatic:
 * $MATH_FACTOR := $REFERENCE|$NUMBER
 */

#include "../parser.hpp"

namespace bonk {

std::unique_ptr<TreeNode> Parser::parse_math_factor() {
    auto result = parse_reference();
    if (result)
        return result;

    if (next_lexeme()->type != LexemeType::l_number) {
        return nullptr;
    }

    auto result_number = std::make_unique<TreeNodeNumber>();
    auto lexeme = std::get<NumberLexeme>(next_lexeme()->data);
    result_number->float_value = lexeme.double_value;
    result_number->integer_value = lexeme.integer_value;
    eat_lexeme();
    return result_number;
}

} // namespace bonk