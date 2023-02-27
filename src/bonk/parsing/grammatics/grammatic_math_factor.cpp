/*
 * Math factor grammatic:
 * $MATH_FACTOR := $REFERENCE|$NUMBER
 */

#include "grammatic_math_factor.hpp"

namespace bonk {

TreeNode* parse_grammatic_math_factor(Parser* parser) {
    TreeNode* result = nullptr;

    result = parse_grammatic_reference(parser);
    if (result)
        return result;

    if (parser->next_lexeme()->type == BONK_LEXEME_NUMBER) {
        result = (TreeNode*)parser->next_lexeme()->number_data.number;
        parser->eat_lexeme();
        return result;
    }

    return nullptr;
}

} // namespace bonk