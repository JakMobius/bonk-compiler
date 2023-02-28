
/*
 * Reference grammatic
 * $REFERENCE = "@" ($VARNAME | "(" $EXPRESSION ")") ("[" $ARGUMENT ("," $ARGUMENT)* "]") | $VARNAME
 * $ARGUMENT = $VARNAME "=" $EXPRESSION
 * $VARNAME = [a-zA-Z_][a-zA-Z0-9_]+
 */

#include "../parser.hpp"

namespace bonk {

TreeNode* Parser::parse_reference() {
    TreeNode* variable = nullptr;

    Lexeme* next = next_lexeme();

    bool is_call = false;

    if (next->type == BONK_LEXEME_CALL) {
        is_call = true;

        eat_lexeme();
        next = next_lexeme();
    }

    if (next->type == BONK_LEXEME_BRACE && next->brace_data.brace_type == BONK_BRACE_L_RB) {
        eat_lexeme();
        variable = parse_expression();

        next = next_lexeme();

        if (next->type != BONK_LEXEME_BRACE || next->brace_data.brace_type != BONK_BRACE_R_RB) {
            if (!linked_compiler->state) {
                error("expected ')'");
            }

            delete variable;
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
        variable = (TreeNode*)next->identifier_data.identifier;
        eat_lexeme();
    }

    if (variable == nullptr) {
        if (is_call) {
            error("expected block name to call");
        }
        return nullptr;
    }

    if (is_call) {
        auto* arguments = parse_arguments();

        auto* call = new TreeNodeCall((TreeNodeIdentifier*)variable, arguments);
        return call;
    }

    return variable;
}

TreeNodeList* Parser::parse_arguments() {

    Lexeme* next = next_lexeme();
    if (next->type != BONK_LEXEME_BRACE || next->brace_data.brace_type != BONK_BRACE_L_SB) {
        return nullptr;
    }

    eat_lexeme();

    auto* argument_list = new TreeNodeList();

    next = next_lexeme();

    while (true) {

        if (next->type != BONK_LEXEME_IDENTIFIER) {
            error("expected parameter name");
            delete argument_list;
            return nullptr;
        }

        TreeNodeIdentifier* parameter_name = next->identifier_data.identifier;

        eat_lexeme();
        next = next_lexeme();

        if (next->type != BONK_LEXEME_OPERATOR ||
            next->operator_data.operator_type != BONK_OPERATOR_ASSIGNMENT) {
            error("expected parameter value");
            delete argument_list;
            return nullptr;
        }

        eat_lexeme();

        TreeNode* parameter_value = parse_expression();
        if (!parameter_value) {
            if (!linked_compiler->state) {
                error("expected parameter value");
            }
            return nullptr;
        }

        auto* argument = new TreeNodeCallParameter(parameter_name, parameter_value);

        argument_list->list.push_back(argument);

        next = next_lexeme();
        if (next->type != BONK_LEXEME_COMMA) {
            break;
        }

        eat_lexeme();
        next = next_lexeme();
    }

    if (next->type != BONK_LEXEME_BRACE || next->brace_data.brace_type != BONK_BRACE_R_SB) {
        delete argument_list;
        error("expected ']'");
        return nullptr;
    }

    eat_lexeme();

    return argument_list;
}

} // namespace bonk