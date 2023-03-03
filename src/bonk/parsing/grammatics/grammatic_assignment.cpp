/*
 * Assignment grammatic
 * $ASSIGNMENT := $IDENTIFIER '=' $EXPRESSION
 */

#include "../parser.hpp"

namespace bonk {

std::unique_ptr<TreeNode> Parser::parse_assignment() {

    Lexeme* lvalue = next_lexeme();
    if (lvalue->type == LexemeType::l_eof)
        return nullptr;

    eat_lexeme();
    Lexeme* assignment = next_lexeme();

    if (assignment->is(OperatorType::o_assign)) {
        // This is not an assignment. Exit silently.
        spit_lexeme();
        return nullptr;
    }

    if (lvalue->type != LexemeType::l_identifier) {
        linked_compiler.error().at(lvalue->start_position) << "expression is not assignable";
        return nullptr;
    }

    auto identifier = std::make_unique<TreeNodeIdentifier>();
    identifier->variable_name = std::get<IdentifierLexeme>(lvalue->data).identifier;
    identifier->source_position = lvalue->start_position;

    eat_lexeme();

    auto rvalue = parse_expression();
    if (!rvalue) {
        if (linked_compiler.state)
            return nullptr;
        linked_compiler.error().at(next_lexeme()->start_position) << "expected value";
        return nullptr;
    }

    auto expression = std::make_unique<TreeNodeOperator>();
    expression->oper_type = OperatorType::o_assign;

    expression->left = std::move(identifier);
    expression->right = std::move(rvalue);

    return expression;
}

} // namespace bonk