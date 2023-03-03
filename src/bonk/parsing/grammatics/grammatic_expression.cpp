/*
 * Expression grammatic:
 * $EXPRESSION := $ASSIGNMENT | $LOGIC_EXPRESSION | $UNARY_OPERATOR
 */

#include "../parser.hpp"

namespace bonk {

std::unique_ptr<TreeNode> Parser::parse_expression() {
    return parse_expression_leveled(false);
}

std::unique_ptr<TreeNode> Parser::parse_expression_leveled(bool top_level) {

    auto expression = parse_logic_expression();
    if (linked_compiler.state)
        return nullptr;

    if (!expression) {
        expression = parse_unary_operator();
        if (linked_compiler.state)
            return nullptr;
    }

    if (top_level) {

        if (!expression) {
            expression = parse_var_definition();
            if (linked_compiler.state)
                return nullptr;
        }

        Lexeme* next = next_lexeme();
        if (next->type != LexemeType::l_semicolon) {
            if (expression) {
                linked_compiler.error().at(next_lexeme()->start_position) << "expected semicolon";
            }
        } else {
            while (next->type == LexemeType::l_semicolon) {
                eat_lexeme();
                next = next_lexeme();
            }
            if (!expression) {
                return parse_expression();
            }
        }
    }

    if (!expression) {
        expression = parse_sub_block();
        if (linked_compiler.state)
            return nullptr;
    }

    return expression;
}

} // namespace bonk