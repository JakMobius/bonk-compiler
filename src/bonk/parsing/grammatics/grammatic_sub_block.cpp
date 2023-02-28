
/*
 * Sub block grammatic
 * $SUB_BLOCK := $CONSIDER_EXPRESSION | $CYCLE_EXPRESSION | "{" $BLOCK "}"
 * $CONSIDER_EXPRESSION := "consider" $EXPR ("{" $BLOCK "}" $OTHERWISE_EXPRESSION) |
 * $OTHERWISE_EXPRESSION $OTHERWISE_EXPRESSION := "otherwise" ($CONSIDER_EXPRESSION | "{" $BLOCK
 * "}") $CYCLE_EXPRESSION := "cycle" "{" $BLOCK "}"
 */

#include "../parser.hpp"

namespace bonk {

TreeNode* Parser::parse_sub_block() {
    TreeNode* expression = parse_cycle();
    if (expression)
        return expression;
    if (linked_compiler->state)
        return nullptr;

    expression = parse_check();
    if (expression)
        return expression;
    if (linked_compiler->state)
        return nullptr;

    return nullptr;
}

TreeNodeCheck* Parser::parse_check() {
    Lexeme* next = next_lexeme();

    if (next->type != BONK_LEXEME_KEYWORD ||
        next->keyword_data.keyword_type != BONK_KEYWORD_CHECK) {
        return nullptr;
    }

    ParserPosition* check_position = next->position;
    eat_lexeme();
    next = next_lexeme();

    TreeNode* condition = parse_expression();
    if (!condition) {
        if (!linked_compiler->state) {
            error("expected expression to check");
        }
        return nullptr;
    }

    auto* block = parse_nested_block();
    if (!block) {
        delete condition;
        if (!linked_compiler->state) {
            error("expected check body");
        }
        return nullptr;
    }

    auto* check = new TreeNodeCheck();

    TreeNodeList* else_block = nullptr;

    next = next_lexeme();

    if (next->type == BONK_LEXEME_KEYWORD && next->keyword_data.keyword_type == BONK_KEYWORD_OR) {
        eat_lexeme();
        else_block = parse_nested_block();
        if (!else_block) {
            delete condition;
            delete block;

            if (!linked_compiler->state) {
                error("expected or body");
            }
            return nullptr;
        }
    }

    check->source_position = check_position->clone();
    check->condition = condition;
    check->check_body = block;
    check->or_body = else_block;

    return check;
}

TreeNodeCycle* Parser::parse_cycle() {
    Lexeme* next = next_lexeme();

    if (next->type != BONK_LEXEME_KEYWORD ||
        next->keyword_data.keyword_type != BONK_KEYWORD_CYCLE) {
        return nullptr;
    }

    ParserPosition* cycle_position = next->position;
    eat_lexeme();

    auto* block = parse_nested_block();
    if (!block) {
        if (!linked_compiler->state)
            error("expected cycle body");
        return nullptr;
    }

    auto* cycle = new TreeNodeCycle();

    cycle->source_position = cycle_position->clone();
    cycle->body = block;
    return cycle;
}

TreeNodeList* Parser::parse_nested_block() {
    Lexeme* next = next_lexeme();

    if (next->type != BONK_LEXEME_BRACE || next->brace_data.brace_type != BONK_BRACE_L_CB) {
        error("expected '{'");
        return nullptr;
    }

    eat_lexeme();

    TreeNodeList* block = parse_block();
    if (!block) {
        if (linked_compiler->state)
            return nullptr;
        error("expected block");
        return nullptr;
    }
    block->source_position = next->position->clone();

    next = next_lexeme();

    if (next->type != BONK_LEXEME_BRACE || next->brace_data.brace_type != BONK_BRACE_R_CB) {
        error("expected '}'");
        delete block;
        return nullptr;
    }

    eat_lexeme();

    return block;
}

} // namespace bonk