
#include "grammatic_logic_expression.hpp"

namespace bonk {

TreeNode* parse_grammatic_logic_expression(Parser* parser) {
    TreeNode* term = parse_grammatic_logic_term(parser);
    TreeNode* result = term;

    do {
        Lexeme* next = parser->next_lexeme();
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

        TreeNode* next_term = parse_grammatic_expression(parser);
        if (!next_term) {
            if (!parser->linked_compiler->state) {
                parser->error("expected expression");
            }
            return nullptr;
        }

        auto* sum = new TreeNodeOperator(oper_type);

        sum->left = result;
        sum->right = next_term;
        result = sum;

    } while (true);

    return result;
}

} // namespace bonk