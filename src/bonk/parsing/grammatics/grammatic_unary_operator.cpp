
#include "../parser.hpp"

namespace bonk {

TreeNode* Parser::parse_bams() {
    Lexeme* lexeme = next_lexeme();
    if (lexeme->type == BONK_LEXEME_INLINE_BAMS) {
        eat_lexeme();

        auto* node = new TreeNodeOperator(BONK_OPERATOR_BAMS);

        node->source_position = lexeme->position->clone();
        node->left = lexeme->identifier_data.identifier;
        return node;
    }

    error("expected inline assembly body");

    return nullptr;
}

TreeNode* Parser::parse_unary_operator() {
    Lexeme* lexeme = next_lexeme();

    if (lexeme->type == BONK_LEXEME_KEYWORD) {

        auto oper = BONK_OPERATOR_INVALID;

        if (lexeme->keyword_data.keyword_type == BONK_KEYWORD_BREK) {
            oper = BONK_OPERATOR_BREK;
        } else if (lexeme->keyword_data.keyword_type == BONK_KEYWORD_REBONK) {
            oper = BONK_OPERATOR_REBONK;
        }

        if (oper != BONK_OPERATOR_INVALID) {
            eat_lexeme();

            auto* statement = new TreeNodeOperator(BONK_OPERATOR_BREK);
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
            eat_lexeme();
            return parse_bams();
        } else {
            return nullptr;
        }

        eat_lexeme();

        TreeNode* expression = parse_expression();
        if (!expression) {
            if (linked_compiler->state)
                return nullptr;
            error("missing operand");
        }

        auto* print_call = new TreeNodeOperator(oper);
        print_call->source_position = lexeme->position->clone();

        print_call->right = expression;

        return print_call;
    }

    return nullptr;
}

} // namespace bonk