
#include "parser.hpp"

namespace bonk {

Parser::Parser(Compiler* compiler) {
    linked_compiler = compiler;
}

TreeNodeList* Parser::parse_file(std::vector<Lexeme>* lexemes) {
    input = lexemes;
    auto* target = new TreeNodeList();

    if (!parse_global(target)) {
        delete target;
        return nullptr;
    }

    return target;
}

bool Parser::append_file(std::vector<Lexeme>* lexemes, TreeNodeList* target) {
    input = lexemes;

    return parse_global(target);
}

void Parser::spit_lexeme() {
    position--;
}

Lexeme* Parser::next_lexeme() {
    return &(*input)[(int)position];
}

void Parser::eat_lexeme() {
    Lexeme* c = next_lexeme();
    assert(c->type);
    position++;
}

} // namespace bonk