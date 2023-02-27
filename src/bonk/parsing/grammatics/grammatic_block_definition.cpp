/*
 * Block definition grammatic
 * $BLOCK_DEFINITION = 'promise'? 'block' $IDENTIFIER '{' $BLOCK '}'
 */

#include "grammatic_block_definition.hpp"

namespace bonk {

TreeNodeBlockDefinition* parse_grammatic_block_definition(Parser* parser) {

    auto old_position = parser->position;
    Lexeme* next = parser->next_lexeme();
    bool is_promised = false;

    if (next->type != BONK_LEXEME_KEYWORD)
        return nullptr;
    if (next->keyword_data.keyword_type == BONK_KEYWORD_PROMISE) {
        parser->eat_lexeme();
        next = parser->next_lexeme();
        if (next->type != BONK_LEXEME_KEYWORD) {
            parser->position = old_position;
            return nullptr;
        }
        is_promised = true;
    }
    if (next->keyword_data.keyword_type != BONK_KEYWORD_BLOCK) {
        parser->position = old_position;
        return nullptr;
    }

    parser->eat_lexeme();
    next = parser->next_lexeme();

    if (next->type != BONK_LEXEME_IDENTIFIER) {
        parser->error("expected block name");
        parser->position = old_position;
        return nullptr;
    }

    TreeNodeIdentifier* block_identifier = next->identifier_data.identifier;

    parser->eat_lexeme();
    next = parser->next_lexeme();

    if (next->type != BONK_LEXEME_BRACE || next->brace_data.brace_type != BONK_BRACE_L_CB) {
        parser->error("expected '{'");
        delete block_identifier;
        parser->position = old_position;
        return nullptr;
    }

    parser->eat_lexeme();

    auto* block = parse_grammatic_block(parser);
    if (!block) {
        delete block_identifier;
        parser->position = old_position;
        return nullptr;
    }

    next = parser->next_lexeme();

    if (next->type != BONK_LEXEME_BRACE || next->brace_data.brace_type != BONK_BRACE_R_CB) {
        parser->error("expected '}'");
        delete block;
        delete block_identifier;
        parser->position = old_position;
        return nullptr;
    }

    parser->eat_lexeme();

    auto* block_definition = new TreeNodeBlockDefinition();

    block_definition->body = block;
    block_definition->block_name = block_identifier;
    block_definition->is_promise = is_promised;

    return block_definition;
}

} // namespace bonk