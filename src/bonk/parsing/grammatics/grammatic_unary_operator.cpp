
#include "grammatic_unary_operator.hpp"

namespace bonk {

tree_node* parse_grammatic_bams(parser* parser) {
    lexeme* lexeme = parser->next_lexeme();
    if (lexeme->type == BONK_LEXEME_INLINE_BAMS) {
        parser->eat_lexeme();

        auto* node = new tree_node_operator(BONK_OPERATOR_BAMS);

        node->source_position = lexeme->position->clone();
        node->left = lexeme->identifier_data.identifier;
        return node;
    }

    parser->error("expected inline assembly body");

    return nullptr;
}

tree_node* parse_grammatic_unary_operator(parser* parser) {
    lexeme* lexeme = parser->next_lexeme();

    if (lexeme->type == BONK_LEXEME_KEYWORD) {

        auto oper = BONK_OPERATOR_INVALID;

        if (lexeme->keyword_data.keyword_type == BONK_KEYWORD_BREK) {
            oper = BONK_OPERATOR_BREK;
        } else if (lexeme->keyword_data.keyword_type == BONK_KEYWORD_REBONK) {
            oper = BONK_OPERATOR_REBONK;
        }

        if (oper != BONK_OPERATOR_INVALID) {
            parser->eat_lexeme();

            auto* statement = new tree_node_operator(BONK_OPERATOR_BREK);
            if (!statement) {
                parser->linked_compiler->out_of_memory();
                return nullptr;
            }

            statement->source_position = lexeme->position->clone();
            return statement;
        }

        if (lexeme->keyword_data.keyword_type == BONK_KEYWORD_PRINT) {
            oper = BONK_OPERATOR_PRINT;
        } else if (lexeme->keyword_data.keyword_type == BONK_KEYWORD_BONK) {
            oper = BONK_OPERATOR_BONK;
        } else if (lexeme->keyword_data.keyword_type == BONK_KEYWORD_REBONK) {
            oper = BONK_OPERATOR_REBONK;
        } else if (lexeme->keyword_data.keyword_type == BONK_KEYWORD_BAMS) {
            parser->eat_lexeme();
            return parse_grammatic_bams(parser);
        } else {
            return nullptr;
        }

        parser->eat_lexeme();

        tree_node* expression = parse_grammatic_expression(parser);
        if (!expression) {
            if (parser->linked_compiler->state)
                return nullptr;
            parser->error("missing operand");
        }

        auto* print_call = new tree_node_operator(oper);
        if (!print_call) {
            parser->linked_compiler->out_of_memory();
            delete expression;
            return nullptr;
        }
        print_call->source_position = lexeme->position->clone();

        print_call->right = expression;

        return print_call;
    }

    return nullptr;
}

} // namespace bonk