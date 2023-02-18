
#include "grammatic_logic_expression.hpp"

namespace bonk {

tree_node* parse_grammatic_logic_expression(parser* parser) {
    tree_node* term = parse_grammatic_logic_term(parser);
    tree_node* result = term;

    do {
        lexeme* next = parser->next_lexeme();
        if (next->type != BONK_LEXEME_KEYWORD)
            break;
        auto oper_type = BONK_OPERATOR_INVALID;

        switch (next->keyword_data.keyword_type) {
        case BONK_KEYWORD_AND:
            oper_type = BONK_OPERATOR_AND;
            break;
        case BONK_KEYWORD_OR:
            oper_type = BONK_OPERATOR_OR;
            break;
        default:
            return result;
        }

        parser->eat_lexeme();

        tree_node* next_term = parse_grammatic_expression(parser);
        if (!next_term) {
            if (!parser->linked_compiler->state) {
                parser->error("expected expression");
            }
            return nullptr;
        }

        auto* sum = new tree_node_operator(oper_type);

        sum->left = result;
        sum->right = next_term;
        result = sum;

    } while (true);

    return result;
}

} // namespace bonk