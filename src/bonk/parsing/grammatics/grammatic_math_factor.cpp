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

    if (next_lexeme()->type != BONK_LEXEME_NUMBER) {
        return nullptr;
    }

    auto result_number = std::make_unique<TreeNodeNumber>();
    result_number->float_value = next_lexeme()->number_data.number.double_value;
    result_number->integer_value = next_lexeme()->number_data.number.integer_value;
    eat_lexeme();
    return result_number;
}

} // namespace bonk