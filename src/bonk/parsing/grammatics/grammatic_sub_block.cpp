
/*
 * Sub block grammatic
 * $SUB_BLOCK := $CONSIDER_EXPRESSION | $CYCLE_EXPRESSION | "{" $BLOCK "}"
 * $CONSIDER_EXPRESSION := "consider" $EXPR ("{" $BLOCK "}" $OTHERWISE_EXPRESSION) |
 * $OTHERWISE_EXPRESSION $OTHERWISE_EXPRESSION := "otherwise" ($CONSIDER_EXPRESSION | "{" $BLOCK
 * "}") $CYCLE_EXPRESSION := "cycle" "{" $BLOCK "}"
 */

#include "grammatic_sub_block.hpp"

namespace bonk {

TreeNode* parse_grammatic_sub_block(Parser* parser) {
    TreeNode* expression = parse_grammatic_cycle(parser);
    if (expression)
        return expression;
    if (parser->linked_compiler->state)
        return nullptr;

    expression = parse_grammatic_check(parser);
    if (expression)
        return expression;
    if (parser->linked_compiler->state)
        return nullptr;

    return nullptr;
}

TreeNodeCheck* parse_grammatic_check(Parser* parser) {
    Lexeme* next = parser->next_lexeme();

    if (next->type != BONK_LEXEME_KEYWORD ||
        next->keyword_data.keyword_type != BONK_KEYWORD_CHECK) {
        return nullptr;
    }

    ParserPosition* check_position = next->position;
    parser->eat_lexeme();
    next = parser->next_lexeme();

    TreeNode* condition = parse_grammatic_expression(parser);
    if (!condition) {
        if (!parser->linked_compiler->state) {
            parser->error("expected expression to check");
        }
        return nullptr;
    }

    auto* block = parse_grammatic_nested_block(parser);
    if (!block) {
        delete condition;
        if (!parser->linked_compiler->state) {
            parser->error("expected check body");
        }
        return nullptr;
    }

    auto* check = new TreeNodeCheck();

    TreeNodeList* else_block = nullptr;

    next = parser->next_lexeme();

    if (next->type == BONK_LEXEME_KEYWORD && next->keyword_data.keyword_type == BONK_KEYWORD_OR) {
        parser->eat_lexeme();
        else_block = parse_grammatic_nested_block(parser);
        if (!else_block) {
            delete condition;
            delete block;

            if (!parser->linked_compiler->state) {
                parser->error("expected or body");
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

TreeNodeCycle* parse_grammatic_cycle(Parser* parser) {
    Lexeme* next = parser->next_lexeme();

    if (next->type != BONK_LEXEME_KEYWORD ||
        next->keyword_data.keyword_type != BONK_KEYWORD_CYCLE) {
        return nullptr;
    }

    ParserPosition* cycle_position = next->position;
    parser->eat_lexeme();

    auto* block = parse_grammatic_nested_block(parser);
    if (!block) {
        if (!parser->linked_compiler->state)
            parser->error("expected cycle body");
        return nullptr;
    }

    auto* cycle = new TreeNodeCycle();

    cycle->source_position = cycle_position->clone();
    cycle->body = block;
    return cycle;
}

TreeNodeList* parse_grammatic_nested_block(Parser* parser) {
    Lexeme* next = parser->next_lexeme();

    if (next->type != BONK_LEXEME_BRACE || next->brace_data.brace_type != BONK_BRACE_L_CB) {
        parser->error("expected '{'");
        return nullptr;
    }

    parser->eat_lexeme();

    TreeNodeList* block = parse_grammatic_block(parser);
    if (!block) {
        if (parser->linked_compiler->state)
            return nullptr;
        parser->error("expected block");
        return nullptr;
    }
    block->source_position = next->position->clone();

    next = parser->next_lexeme();

    if (next->type != BONK_LEXEME_BRACE || next->brace_data.brace_type != BONK_BRACE_R_CB) {
        parser->error("expected '}'");
        delete block;
        return nullptr;
    }

    parser->eat_lexeme();

    return block;
}

} // namespace bonk