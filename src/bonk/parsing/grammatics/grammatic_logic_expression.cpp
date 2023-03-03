
#include "../parser.hpp"

namespace bonk {

std::unique_ptr<TreeNode> Parser::parse_logic_expression() {

    auto result = parse_logic_term();

    do {
        Lexeme* next = next_lexeme();
        if (next->type != LexemeType::l_operator)
            break;
        auto oper_type = std::get<OperatorLexeme>(next->data).type;

        eat_lexeme();

        auto next_term = parse_expression();
        if (!next_term) {
            if (!linked_compiler.state) {
                linked_compiler.error().at(next_lexeme()->start_position) << "expected expression";
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