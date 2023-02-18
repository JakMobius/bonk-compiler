/*
 * Global grammatic:
 * $GLOBAL := ($PROGRAM) '\0'
 */

#include "grammatic_global.hpp"

namespace bonk {

bool parse_grammatic_global(parser* parser, tree_node_list<tree_node*>* target) {

    while (true) {
        unsigned long old_position = parser->position;

        if (!parse_grammatic_help(parser, target)) {
            if (old_position == parser->position)
                break;
            continue;
        }
    }

    if (!parse_grammatic_program(parser, target)) {
        if (!parser->linked_compiler->state)
            parser->error("empty program");
        return false;
    }

    lexeme* next = parser->next_lexeme();

    if (next->type != BONK_LEXEME_NULL) {
        if (!parser->linked_compiler->state)
            parser->error("expected end of file");
        return false;
    }

    return true;
}

} // namespace bonk