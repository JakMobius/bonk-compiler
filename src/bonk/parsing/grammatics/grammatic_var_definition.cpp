
/*
 * Var definition grammatic
 * Var_Definition { "var" Identifier }
 */
#include "../parser.hpp"

namespace bonk {

std::unique_ptr<TreeNodeVariableDefinition> Parser::parse_var_definition() {

    Lexeme* next = next_lexeme();
    bool is_contextual = false;

    if (next->type != BONK_LEXEME_KEYWORD)
        return nullptr;

    if (next->keyword_data.keyword_type == BONK_KEYWORD_CONTEXT) {
        is_contextual = true;

        eat_lexeme();
        next = next_lexeme();
        if (next->type != BONK_LEXEME_KEYWORD)
            return nullptr;
    }

    if (next->keyword_data.keyword_type != BONK_KEYWORD_VAR)
        return nullptr;

    eat_lexeme();
    next = next_lexeme();

    if (next->type != BONK_LEXEME_IDENTIFIER) {
        error("expected variable name");
        return nullptr;
    }

    eat_lexeme();

    auto identifier = std::make_unique<TreeNodeIdentifier>();
    identifier->variable_name = next->identifier_data.identifier;
    identifier->source_position = next->position->clone();

    auto definition =
        std::make_unique<TreeNodeVariableDefinition>();
    definition->variable_name = std::move(identifier);
    definition->is_contextual = is_contextual;

    definition->source_position = next->position->clone();

    next = next_lexeme();

    if (next->type == BONK_LEXEME_OPERATOR &&
        next->operator_data.operator_type == BONK_OPERATOR_ASSIGNMENT) {
        eat_lexeme();
        if (is_contextual) {
            error("context variable may not be initialized");
            return nullptr;
        }

        definition->variable_value = parse_expression();

        if (definition->variable_value == nullptr) {
            if (!linked_compiler->state) {
                error("expected initial variable value");
            }
            return nullptr;
        }
    }

    return definition;
}

} // namespace bonk