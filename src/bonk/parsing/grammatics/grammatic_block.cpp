/*
 * Block grammatic
 * $BLOCK := $EXPRESSION*
 */

#include "../parser.hpp"

namespace bonk {

std::unique_ptr<TreeNode> Parser::parse_code_line() {
    auto expression = parse_expression_leveled(true);
    if (expression)
        return expression;
    if (linked_compiler.state)
        return nullptr;

    return nullptr;
}

std::unique_ptr<TreeNodeList> Parser::parse_block() {

    auto result = std::make_unique<TreeNodeList>();

    do {
        auto block = parse_code_line();
        if (linked_compiler.state)
            return nullptr;

        if (!block)
            break;

        result->list.push_back(std::move(block));
    } while (true);

    return result;
}

} // namespace bonk