/*
 * Block definition grammatic
 * $BLOCK_DEFINITION = 'promise'? 'block' $IDENTIFIER '{' $BLOCK '}'
 */

#include "../parser.hpp"

namespace bonk {

std::unique_ptr<TreeNodeBlockDefinition> Parser::parse_block_definition() {

    auto old_position = position;
    auto next = next_lexeme();
    bool is_promised = false;

    if (next->type != BONK_LEXEME_KEYWORD)
        return nullptr;
    if (next->keyword_data.keyword_type == BONK_KEYWORD_PROMISE) {
        eat_lexeme();
        next = next_lexeme();
        if (next->type != BONK_LEXEME_KEYWORD) {
            position = old_position;
            return nullptr;
        }
        is_promised = true;
    }
    if (next->keyword_data.keyword_type != BONK_KEYWORD_BLOCK) {
        position = old_position;
        return nullptr;
    }

    eat_lexeme();
    next = next_lexeme();

    if (next->type != BONK_LEXEME_IDENTIFIER) {
        error("expected block name");
        position = old_position;
        return nullptr;
    }

    auto block_identifier = std::make_unique<TreeNodeIdentifier>();
    block_identifier->variable_name = next->identifier_data.identifier;
    block_identifier->source_position = next->position->clone();

    eat_lexeme();
    next = next_lexeme();

    if (next->type != BONK_LEXEME_BRACE || next->brace_data.brace_type != BONK_BRACE_L_CB) {
        error("expected '{'");
        position = old_position;
        return nullptr;
    }

    eat_lexeme();

    auto block = parse_block();
    if (!block) {
        position = old_position;
        return nullptr;
    }

    next = next_lexeme();

    if (next->type != BONK_LEXEME_BRACE || next->brace_data.brace_type != BONK_BRACE_R_CB) {
        error("expected '}'");
        position = old_position;
        return nullptr;
    }

    eat_lexeme();

    auto block_definition = std::make_unique<TreeNodeBlockDefinition>();

    block_definition->body = std::move(block);
    block_definition->block_name = std::move(block_identifier);
    block_definition->is_promise = is_promised;

    return block_definition;
}

} // namespace bonk