/*
 * Global grammatic:
 * $GLOBAL := ($PROGRAM) '\0'
 */

#include "../parser.hpp"

namespace bonk {

bool Parser::parse_global(TreeNodeList* target) {

    while (true) {
        unsigned long old_position = position;

        if (!parse_help(target)) {
            if (old_position == position)
                break;
            continue;
        }
    }

    if (!parse_program(target)) {
        if (!linked_compiler.state)
            linked_compiler.error() << "empty program";
        return false;
    }

    Lexeme* next = next_lexeme();

    if (next->type != LexemeType::l_eof) {
        if (!linked_compiler.state)
            linked_compiler.error().at(next_lexeme()->start_position) << "expected end of file";
        return false;
    }

    return true;
}

} // namespace bonk