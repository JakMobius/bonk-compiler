/*
 * Program grammatic
 * $PROGRAM = ($BLOCK_DEFINITION | $VAR_DEFINITION)+
 */

#include "grammatic_program.hpp"

namespace bonk {

TreeNode* parse_grammatic_global_definition(Parser* parser) {
    TreeNode* expression = parse_grammatic_block_definition(parser);
    if (expression)
        return expression;
    if (parser->linked_compiler->state)
        return nullptr;

    TreeNodeVariableDefinition* variable_definition = parse_grammatic_var_definition(parser);
    if (variable_definition) {
        if (variable_definition->is_contextual) {
            parser->error("global variables may not be contextual");
            delete variable_definition;
            return nullptr;
        }
        Lexeme* next = parser->next_lexeme();
        if (next->type != BONK_LEXEME_SEMICOLON) {
            parser->error("expected semicolon");
            delete variable_definition;
            return nullptr;
        }
        parser->eat_lexeme();
        return variable_definition;
    }

    return nullptr;
}

bool parse_grammatic_program(Parser* parser, TreeNodeList<TreeNode*>* target) {

    TreeNode* block = parse_grammatic_global_definition(parser);
    if (!block) {
        return false;
    }

    while (block) {
        if (target->list.insert_tail(block)) {
            parser->linked_compiler->out_of_memory();
            return false;
        }

        block = parse_grammatic_global_definition(parser);
    }

    return true;
}

} // namespace bonk