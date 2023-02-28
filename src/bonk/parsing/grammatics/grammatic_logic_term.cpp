
#include "../parser.hpp"

namespace bonk {

TreeNode* Parser::parse_logic_term() {

    TreeNode* expression = parse_unary_operator();
    if (expression)
        return expression;
    if (linked_compiler->state)
        return nullptr;

    expression = parse_assignment();
    if (expression)
        return expression;
    if (linked_compiler->state)
        return nullptr;

    expression = parse_comparison();
    if (expression)
        return expression;
    if (linked_compiler->state)
        return nullptr;

    return expression;
}

} // namespace bonk
