
#include "../parser.hpp"

namespace bonk {

TreeNode* Parser::parse_logic_expression() {
    TreeNode* term = parse_logic_term();
    TreeNode* result = term;

    do {
        Lexeme* next = next_lexeme();
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

        eat_lexeme();

        TreeNode* next_term = parse_expression();
        if (!next_term) {
            if (!linked_compiler->state) {
                error("expected expression");
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