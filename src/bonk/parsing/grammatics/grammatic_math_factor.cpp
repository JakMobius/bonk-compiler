/*
 * Math factor grammatic:
 * $MATH_FACTOR := $REFERENCE|$NUMBER
 */

#include "grammatic_math_factor.hpp"

namespace bonk {

tree_node* parse_grammatic_math_factor(parser* parser) {
    tree_node* result = nullptr;

    result = parse_grammatic_reference(parser);
    if (result)
        return result;

    if (parser->next_lexeme()->type == BONK_LEXEME_NUMBER) {
        result = (tree_node*)parser->next_lexeme()->number_data.number;
        parser->eat_lexeme();
        return result;
    }

    return nullptr;
}

} // namespace bonk