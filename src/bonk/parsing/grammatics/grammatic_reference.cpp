
/*
 * Reference grammatic
 * $REFERENCE = "@" ($VARNAME | "(" $EXPRESSION ")") ("[" $ARGUMENT ("," $ARGUMENT)* "]") | $VARNAME
 * $ARGUMENT = $VARNAME "=" $EXPRESSION
 * $VARNAME = [a-zA-Z_][a-zA-Z0-9_]+
 */

#include "../parser.hpp"

namespace bonk {

std::unique_ptr<TreeNode> Parser::parse_reference() {
    std::unique_ptr<TreeNode> variable = nullptr;

    Lexeme* next = next_lexeme();

    bool is_call = false;

    if (next->is(OperatorType::o_call)) {
        is_call = true;

        eat_lexeme();
        next = next_lexeme();
    }

    if (next->is(BraceType('('))) {
        eat_lexeme();

        if (is_call) {
            linked_compiler.error().at(next_lexeme()->start_position) << "cannot call an expression";
            return nullptr;
        }

        variable = parse_expression();
        next = next_lexeme();

        if (next->is(BraceType(')'))) {
            if (!linked_compiler.state) {
                linked_compiler.error().at(next_lexeme()->start_position) << "expected ')'";
            }
            return nullptr;
        }
        if (variable == nullptr) {
            if (!linked_compiler.state) {
                linked_compiler.error().at(next_lexeme()->start_position) << "empty parenthesis in expression context";
            }

            eat_lexeme();
            return nullptr;
        }
        eat_lexeme();
    } else if (next->is_identifier()) {
        auto identifier = std::make_unique<TreeNodeIdentifier>();
        identifier->variable_name = std::get<IdentifierLexeme>(next->data).identifier;
        identifier->source_position = next->start_position;
        eat_lexeme();

        if (is_call) {
            auto arguments = parse_arguments();

            auto call = std::make_unique<TreeNodeCall>();
            call->source_position = identifier->source_position;
            call->call_function = std::move(identifier);
            call->call_parameters = std::move(arguments);

            return call;
        }

        variable = std::move(identifier);
    }

    if (variable == nullptr) {
        if (is_call) {
            linked_compiler.error().at(next_lexeme()->start_position) << "expected block name to call";
        }
        return nullptr;
    }

    return variable;
}

std::unique_ptr<TreeNodeList> Parser::parse_arguments() {

    Lexeme* next = next_lexeme();
    if (next->is(BraceType('['))) {
        return nullptr;
    }

    eat_lexeme();

    auto argument_list = std::make_unique<TreeNodeList>();

    next = next_lexeme();

    while (true) {

        if (!next->is_identifier()) {
            linked_compiler.error().at(next_lexeme()->start_position) << "expected parameter name";
            return nullptr;
        }

        auto parameter_name = std::make_unique<TreeNodeIdentifier>();
        parameter_name->variable_name = std::get<IdentifierLexeme>(next->data).identifier;
        parameter_name->source_position = next->start_position;

        eat_lexeme();
        next = next_lexeme();

        if (!next->is(OperatorType::o_assign)) {
            linked_compiler.error().at(next_lexeme()->start_position) << "expected parameter value";
            return nullptr;
        }

        eat_lexeme();

        auto parameter_value = parse_expression();
        if (!parameter_value) {
            if (!linked_compiler.state) {
                linked_compiler.error().at(next_lexeme()->start_position) << "expected parameter value";
            }
            return nullptr;
        }

        auto argument = std::make_unique<TreeNodeCallParameter>();
        argument->parameter_name = std::move(parameter_name);
        argument->parameter_value = std::move(parameter_value);

        argument_list->list.push_back(std::move(argument));

        next = next_lexeme();
        if (next->type != LexemeType::l_comma) {
            break;
        }

        eat_lexeme();
        next = next_lexeme();
    }

    if (!next->is(BraceType(']'))) {
        linked_compiler.error().at(next_lexeme()->start_position) << "expected ']'";
        return nullptr;
    }

    eat_lexeme();

    return argument_list;
}

} // namespace bonk