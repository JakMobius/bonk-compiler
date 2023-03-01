
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

    if (next->type == BONK_LEXEME_CALL) {
        is_call = true;

        eat_lexeme();
        next = next_lexeme();
    }

    if (next->type == BONK_LEXEME_BRACE && next->brace_data.brace_type == BONK_BRACE_L_RB) {
        eat_lexeme();

        if (is_call) {
            error("cannot call an expression");
            return nullptr;
        }

        variable = parse_expression();
        next = next_lexeme();

        if (next->type != BONK_LEXEME_BRACE || next->brace_data.brace_type != BONK_BRACE_R_RB) {
            if (!linked_compiler->state) {
                error("expected ')'");
            }
            return nullptr;
        }
        if (variable == nullptr) {
            if (!linked_compiler->state) {
                error("empty parenthesis in expression context");
            }

            eat_lexeme();
            return nullptr;
        }
        eat_lexeme();
    } else if (next->type == BONK_LEXEME_IDENTIFIER) {
        auto identifier = std::make_unique<TreeNodeIdentifier>();
        identifier->variable_name = next->identifier_data.identifier;
        identifier->source_position = next->position;
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
            error("expected block name to call");
        }
        return nullptr;
    }

    return variable;
}

std::unique_ptr<TreeNodeList> Parser::parse_arguments() {

    Lexeme* next = next_lexeme();
    if (next->type != BONK_LEXEME_BRACE || next->brace_data.brace_type != BONK_BRACE_L_SB) {
        return nullptr;
    }

    eat_lexeme();

    auto argument_list = std::make_unique<TreeNodeList>();

    next = next_lexeme();

    while (true) {

        if (next->type != BONK_LEXEME_IDENTIFIER) {
            error("expected parameter name");
            return nullptr;
        }

        auto parameter_name = std::make_unique<TreeNodeIdentifier>();
        parameter_name->variable_name = next->identifier_data.identifier;
        parameter_name->source_position = next->position;

        eat_lexeme();
        next = next_lexeme();

        if (next->type != BONK_LEXEME_OPERATOR ||
            next->operator_data.operator_type != BONK_OPERATOR_ASSIGNMENT) {
            error("expected parameter value");
            return nullptr;
        }

        eat_lexeme();

        auto parameter_value = parse_expression();
        if (!parameter_value) {
            if (!linked_compiler->state) {
                error("expected parameter value");
            }
            return nullptr;
        }

        auto argument = std::make_unique<TreeNodeCallParameter>();
        argument->parameter_name = std::move(parameter_name);
        argument->parameter_value = std::move(parameter_value);

        argument_list->list.push_back(std::move(argument));

        next = next_lexeme();
        if (next->type != BONK_LEXEME_COMMA) {
            break;
        }

        eat_lexeme();
        next = next_lexeme();
    }

    if (next->type != BONK_LEXEME_BRACE || next->brace_data.brace_type != BONK_BRACE_R_SB) {
        error("expected ']'");
        return nullptr;
    }

    eat_lexeme();

    return argument_list;
}

} // namespace bonk