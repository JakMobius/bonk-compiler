/*
 * Block grammatic
 * $BLOCK := $EXPRESSION*
 */

#include "../parser.hpp"

namespace bonk {

TreeNode* Parser::parse_code_line() {
    TreeNode* expression = parse_expression_leveled(true);
    if (expression)
        return expression;
    if (linked_compiler->state)
        return nullptr;

    return nullptr;
}

TreeNodeList* Parser::parse_block() {

    auto* list = new TreeNodeList();

    auto* result = list;

    do {
        TreeNode* block = parse_code_line();
        if (linked_compiler->state)
            return nullptr;

        if (!block)
            break;

        list->list.push_back(block);
    } while (true);

    return result;
}

} // namespace bonk