/*
 * Block definition grammatic
 * $BLOCK_DEFINITION = 'block' $IDENTIFIER '{' $BLOCK '}'
 */

#include "../parser.hpp"

namespace bonk {

std::unique_ptr<TreeNodeBlockDefinition> Parser::parse_block_definition() {

    auto old_position = position;
    auto next = next_lexeme();

    if (!next->is(OperatorType::o_blok)) {
        return nullptr;
    }

    eat_lexeme();
    next = next_lexeme();

    if (next->is_identifier()) {
        linked_compiler.error().at(next_lexeme()->start_position) << "expected block name";
        position = old_position;
        return nullptr;
    }

    auto block_identifier = std::make_unique<TreeNodeIdentifier>();
    block_identifier->variable_name = std::get<IdentifierLexeme>(next->data).identifier;
    block_identifier->source_position = next->start_position;

    eat_lexeme();
    next = next_lexeme();

    if (next->is(BraceType('{'))) {
        linked_compiler.error().at(next_lexeme()->start_position) << "expected '{'";
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

    if (next->is(BraceType('}'))) {
        linked_compiler.error().at(next_lexeme()->start_position) << "expected '}'";
        position = old_position;
        return nullptr;
    }

    eat_lexeme();

    auto block_definition = std::make_unique<TreeNodeBlockDefinition>();

    block_definition->body = std::move(block);
    block_definition->block_name = std::move(block_identifier);

    return block_definition;
}

} // namespace bonk