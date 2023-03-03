
#include "../parser.hpp"

namespace bonk {

std::unique_ptr<TreeNode> Parser::parse_logic_term() {

    auto expression = parse_unary_operator();
    if (expression)
        return expression;
    if (linked_compiler.state)
        return nullptr;

    expression = parse_assignment();
    if (expression)
        return expression;
    if (linked_compiler.state)
        return nullptr;

    expression = parse_comparison();
    if (expression)
        return expression;
    if (linked_compiler.state)
        return nullptr;

    return expression;
}

} // namespace bonk
