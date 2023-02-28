/*
 * Program grammatic
 * $PROGRAM = ($BLOCK_DEFINITION | $VAR_DEFINITION)+
 */

#include "../parser.hpp"

namespace bonk {

TreeNode* Parser::parse_global_definition() {
    TreeNode* expression = parse_block_definition();
    if (expression)
        return expression;
    if (linked_compiler->state)
        return nullptr;

    TreeNodeVariableDefinition* variable_definition = parse_var_definition();
    if (variable_definition) {
        if (variable_definition->is_contextual) {
            error("global variables may not be contextual");
            delete variable_definition;
            return nullptr;
        }
        Lexeme* next = next_lexeme();
        if (next->type != BONK_LEXEME_SEMICOLON) {
            error("expected semicolon");
            delete variable_definition;
            return nullptr;
        }
        eat_lexeme();
        return variable_definition;
    }

    return nullptr;
}

bool Parser::parse_program(TreeNodeList* target) {

    TreeNode* block = parse_global_definition();
    if (!block) {
        return false;
    }

    while (block) {
        target->list.push_back(block);

        block = parse_global_definition();
    }

    return true;
}

} // namespace bonk