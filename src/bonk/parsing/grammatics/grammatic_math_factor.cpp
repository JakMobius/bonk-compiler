/*
 * Math factor grammatic:
 * $MATH_FACTOR := $REFERENCE|$NUMBER
 */

#include "../parser.hpp"

namespace bonk {

TreeNode* Parser::parse_math_factor() {
    TreeNode* result = parse_reference();
    if (result)
        return result;

    if (next_lexeme()->type == BONK_LEXEME_NUMBER) {
        result = (TreeNode*)next_lexeme()->number_data.number;
        eat_lexeme();
        return result;
    }

    return nullptr;
}

} // namespace bonk