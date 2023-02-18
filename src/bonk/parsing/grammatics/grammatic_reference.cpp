
/*
 * Reference grammatic
 * $REFERENCE = "@" ($VARNAME | "(" $EXPRESSION ")") ("[" $ARGUMENT ("," $ARGUMENT)* "]") | $VARNAME
 * $ARGUMENT = $VARNAME "=" $EXPRESSION
 * $VARNAME = [a-zA-Z_][a-zA-Z0-9_]+
 */

#include "grammatic_reference.hpp"

namespace bonk {

tree_node* parse_grammatic_reference(parser* parser) {
    tree_node* variable = nullptr;

    lexeme* next = parser->next_lexeme();

    bool is_call = false;

    if (next->type == BONK_LEXEME_CALL) {
        is_call = true;

        parser->eat_lexeme();
        next = parser->next_lexeme();
    }

    if (next->type == BONK_LEXEME_BRACE && next->brace_data.brace_type == BONK_BRACE_L_RB) {
        parser->eat_lexeme();
        variable = parse_grammatic_expression(parser);

        next = parser->next_lexeme();

        if (next->type != BONK_LEXEME_BRACE || next->brace_data.brace_type != BONK_BRACE_R_RB) {
            if (!parser->linked_compiler->state) {
                parser->error("expected ')'");
            }

            delete variable;
            return nullptr;
        }
        if (variable == nullptr) {
            if (!parser->linked_compiler->state) {
                parser->error("empty parenthesis in expression context");
            }

            parser->eat_lexeme();
            return nullptr;
        }
        parser->eat_lexeme();
    } else if (next->type == BONK_LEXEME_IDENTIFIER) {
        variable = (tree_node*)next->identifier_data.identifier;
        parser->eat_lexeme();
    }

    if (variable == nullptr) {
        if (is_call) {
            parser->error("expected block name to call");
        }
        return nullptr;
    }

    if (is_call) {
        auto* arguments = parse_grammatic_arguments(parser);

        auto* call = new tree_node_call((tree_node_identifier*)variable, arguments);
        if (!call) {
            parser->linked_compiler->out_of_memory();
            return nullptr;
        }
        return call;
    }

    return variable;
}

tree_node_list<tree_node_call_parameter*>* parse_grammatic_arguments(parser* parser) {

    lexeme* next = parser->next_lexeme();
    if (next->type != BONK_LEXEME_BRACE || next->brace_data.brace_type != BONK_BRACE_L_SB) {
        return nullptr;
    }

    parser->eat_lexeme();

    auto* argument_list = new tree_node_list<tree_node_call_parameter*>();
    if (!argument_list) {
        parser->linked_compiler->out_of_memory();
        return nullptr;
    }

    next = parser->next_lexeme();

    while (true) {

        if (next->type != BONK_LEXEME_IDENTIFIER) {
            parser->error("expected parameter name");
            delete argument_list;
            return nullptr;
        }

        tree_node_identifier* parameter_name = next->identifier_data.identifier;

        parser->eat_lexeme();
        next = parser->next_lexeme();

        if (next->type != BONK_LEXEME_OPERATOR ||
            next->operator_data.operator_type != BONK_OPERATOR_ASSIGNMENT) {
            parser->error("expected parameter value");
            delete argument_list;
            return nullptr;
        }

        parser->eat_lexeme();

        tree_node* parameter_value = parse_grammatic_expression(parser);
        if (!parameter_value) {
            if (!parser->linked_compiler->state) {
                parser->error("expected parameter value");
            }
            return nullptr;
        }

        auto* argument = new tree_node_call_parameter(parameter_name, parameter_value);

        if (!argument || argument_list->list.insert_tail(argument)) {
            delete argument_list;
            parser->linked_compiler->out_of_memory();
            return nullptr;
        }

        next = parser->next_lexeme();
        if (next->type != BONK_LEXEME_COMMA) {
            break;
        }

        parser->eat_lexeme();
        next = parser->next_lexeme();
    }

    if (next->type != BONK_LEXEME_BRACE || next->brace_data.brace_type != BONK_BRACE_R_SB) {
        delete argument_list;
        parser->error("expected ']'");
        return nullptr;
    }

    parser->eat_lexeme();

    return argument_list;
}

} // namespace bonk