/*
 * Block grammatic
 * $BLOCK := $EXPRESSION*
 */

#include "grammatic_block.hpp"

namespace bonk {

tree_node* parse_code_line(parser* parser) {
    tree_node* expression = parse_grammatic_expression_leveled(parser, true);
    if (expression)
        return expression;
    if (parser->linked_compiler->state)
        return nullptr;

    return nullptr;
}

tree_node_list<tree_node*>* parse_grammatic_block(parser* parser) {

    auto* list = new tree_node_list<tree_node*>();

    if (!list) {
        parser->linked_compiler->out_of_memory();
        return nullptr;
    }

    auto* result = list;

    do {
        tree_node* block = parse_code_line(parser);
        if (parser->linked_compiler->state)
            return nullptr;

        if (!block)
            break;

        if (list->list.insert_tail(block)) {
            parser->linked_compiler->out_of_memory();
            delete list;
            return nullptr;
        }
    } while (true);

    return result;
}

} // namespace bonk