/*
 * Program grammatic
 * $PROGRAM = ($BLOCK_DEFINITION | $VAR_DEFINITION)+
 */

#include "../parser.hpp"

namespace bonk {

std::unique_ptr<TreeNode> Parser::parse_global_definition() {
    auto expression = parse_block_definition();
    if (expression)
        return expression;
    if (linked_compiler.state)
        return nullptr;

    auto variable_definition = parse_var_definition();
    if (variable_definition) {
        if (variable_definition->is_contextual) {
            linked_compiler.error().at(variable_definition->source_position) << "global variables may not be contextual";
            return nullptr;
        }
        Lexeme* next = next_lexeme();
        if (next->type != LexemeType::l_semicolon) {
            linked_compiler.error().at(next_lexeme()->start_position) << "expected semicolon";
            return nullptr;
        }
        eat_lexeme();
        return variable_definition;
    }

    return nullptr;
}

bool Parser::parse_program(TreeNodeList* target) {

    auto block = parse_global_definition();
    if (!block) {
        return false;
    }

    while (block) {
        target->list.push_back(std::move(block));

        block = parse_global_definition();
    }

    return true;
}

} // namespace bonk