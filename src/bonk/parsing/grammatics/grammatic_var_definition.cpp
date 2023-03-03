
/*
 * Var definition grammatic
 * Var_Definition { "var" Identifier }
 */
#include "../parser.hpp"

namespace bonk {

std::unique_ptr<TreeNodeVariableDefinition> Parser::parse_var_definition() {

    Lexeme* next = next_lexeme();
    bool is_contextual = false;

    if (!next->is(OperatorType::o_bowl))
        return nullptr;

    eat_lexeme();
    next = next_lexeme();

    if (next->is_identifier()) {
        linked_compiler.error().at(next->start_position) << "expected variable name";
        return nullptr;
    }

    eat_lexeme();

    auto identifier = std::make_unique<TreeNodeIdentifier>();

    identifier->variable_name = std::get<IdentifierLexeme>(next->data).identifier;
    identifier->source_position = next->start_position;

    auto definition = std::make_unique<TreeNodeVariableDefinition>();
    definition->variable_name = std::move(identifier);
    definition->is_contextual = is_contextual;

    definition->source_position = next->start_position;

    next = next_lexeme();

    if (!next->is(OperatorType::o_assign))
        return definition;
    eat_lexeme();

    definition->variable_value = parse_expression();

    if (definition->variable_value == nullptr) {
        if (!linked_compiler.state) {
            linked_compiler.error().at(next_lexeme()->start_position)
                << "expected initial variable value";
        }
        return nullptr;
    }

    return definition;
}

} // namespace bonk