
#include "parser.hpp"
#include "../compiler.hpp"

namespace bonk {

Parser::Parser(Compiler& compiler): linked_compiler(compiler) {

}

std::unique_ptr<TreeNodeList> Parser::parse_file(std::vector<Lexeme>* lexemes) {
    input = lexemes;
    auto target = std::make_unique<TreeNodeList>();

    if (!parse_global(target.get())) {
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
    assert(c->type != LexemeType::l_eof);
    position++;
}

} // namespace bonk