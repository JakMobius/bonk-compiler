
#include "../parser.hpp"

namespace bonk {

std::unique_ptr<TreeNode> Parser::parse_unary_operator() {
    Lexeme* lexeme = next_lexeme();

    if (lexeme->type != LexemeType::l_operator) {
        return nullptr;
    }
    auto oper = std::get<OperatorLexeme>(lexeme->data).type;

    switch(oper) {
    case OperatorType::o_brek: {
        eat_lexeme();
        auto no_arg_operator = std::make_unique<TreeNodeOperator>();
        no_arg_operator->oper_type = OperatorType::o_brek;
        no_arg_operator->source_position = lexeme->start_position;
        return no_arg_operator;
    }
    case OperatorType::o_bonk:
    case OperatorType::o_call: {
        eat_lexeme();
        auto expression = parse_expression();
        if (!expression) {
            if (linked_compiler.state)
                return nullptr;
            linked_compiler.error().at(next_lexeme()->start_position) << "missing operand";
        }

        auto unary_operator = std::make_unique<TreeNodeOperator>();
        unary_operator->oper_type = oper;
        unary_operator->source_position = lexeme->start_position;
        unary_operator->right = std::move(expression);

        return unary_operator;
    }
    default:
        return nullptr;
    }
}

} // namespace bonk