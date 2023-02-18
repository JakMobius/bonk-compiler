
#include "grammatic_logic_term.hpp"

namespace bonk {

tree_node* parse_grammatic_logic_term(parser* parser) {

    tree_node* expression = parse_grammatic_unary_operator(parser);
    if (expression)
        return expression;
    if (parser->linked_compiler->state)
        return nullptr;

    expression = parse_grammatic_assignment(parser);
    if (expression)
        return expression;
    if (parser->linked_compiler->state)
        return nullptr;

    expression = parse_grammatic_comparation(parser);
    if (expression)
        return expression;
    if (parser->linked_compiler->state)
        return nullptr;

    return expression;
}

} // namespace bonk
