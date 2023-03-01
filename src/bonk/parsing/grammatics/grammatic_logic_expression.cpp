
#include "../parser.hpp"

namespace bonk {

std::unique_ptr<TreeNode> Parser::parse_logic_expression() {

    auto result = parse_logic_term();

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

        auto next_term = parse_expression();
        if (!next_term) {
            if (!linked_compiler->state) {
                error("expected expression");
            }
            return nullptr;
        }

        auto sum = std::make_unique<TreeNodeOperator>();
        sum->oper_type = oper_type;

        sum->left = std::move(result);
        sum->right = std::move(next_term);
        result = std::move(sum);

    } while (true);

    return result;
}

} // namespace bonk