
/*
 * Sub block grammatic
 * $SUB_BLOCK := $CONSIDER_EXPRESSION | $CYCLE_EXPRESSION | "{" $BLOCK "}"
 * $CONSIDER_EXPRESSION := "consider" $EXPR ("{" $BLOCK "}" $OTHERWISE_EXPRESSION) |
 * $OTHERWISE_EXPRESSION $OTHERWISE_EXPRESSION := "otherwise" ($CONSIDER_EXPRESSION | "{" $BLOCK
 * "}") $CYCLE_EXPRESSION := "cycle" "{" $BLOCK "}"
 */

#include "../parser.hpp"

namespace bonk {

std::unique_ptr<TreeNode> Parser::parse_sub_block() {
    std::unique_ptr<TreeNode> expression = parse_loop();
    if (expression)
        return expression;
    return nullptr;
}

std::unique_ptr<TreeNodeCycle> Parser::parse_loop() {
    Lexeme* next = next_lexeme();

    if (!next->is(OperatorType::o_loop)) {
        return nullptr;
    }

    const ParserPosition& cycle_position = next->start_position;
    eat_lexeme();

    auto block = parse_nested_block();
    if (!block) {
        if (!linked_compiler.state)
            linked_compiler.error().at(next_lexeme()->start_position) << "expected cycle body";
        return nullptr;
    }

    auto cycle = std::make_unique<TreeNodeCycle>();
    cycle->source_position = cycle_position;
    cycle->body = std::move(block);
    return cycle;
}

std::unique_ptr<TreeNodeList> Parser::parse_nested_block() {
    Lexeme* next = next_lexeme();

    if(!next->is(BraceType('{'))) {
        return nullptr;
    }

    eat_lexeme();

    auto block = parse_block();
    if (!block) {
        if (linked_compiler.state)
            return nullptr;
        linked_compiler.error().at(next_lexeme()->start_position) << "expected block";
        return nullptr;
    }
    block->source_position = next->start_position;

    next = next_lexeme();

    if (!next->is(BraceType('}'))) {
        linked_compiler.error().at(next_lexeme()->start_position) << "expected '}'";
        return nullptr;
    }

    eat_lexeme();

    return block;
}

} // namespace bonk