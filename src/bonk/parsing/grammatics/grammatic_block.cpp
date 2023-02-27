/*
 * Block grammatic
 * $BLOCK := $EXPRESSION*
 */

#include "grammatic_block.hpp"

namespace bonk {

TreeNode* parse_code_line(Parser* parser) {
    TreeNode* expression = parse_grammatic_expression_leveled(parser, true);
    if (expression)
        return expression;
    if (parser->linked_compiler->state)
        return nullptr;

    return nullptr;
}

TreeNodeList<TreeNode*>* parse_grammatic_block(Parser* parser) {

    auto* list = new TreeNodeList<TreeNode*>();

    if (!list) {
        parser->linked_compiler->out_of_memory();
        return nullptr;
    }

    auto* result = list;

    do {
        TreeNode* block = parse_code_line(parser);
        if (parser->linked_compiler->state)
            return nullptr;

        if (!block)
            break;

        list->list.push_back(block);
    } while (true);

    return result;
}

} // namespace bonk