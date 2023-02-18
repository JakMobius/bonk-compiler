
#include "parser.hpp"

namespace bonk {

parser::parser(compiler* compiler) {
    linked_compiler = compiler;
    input = nullptr;
}

tree_node_list<tree_node*>* parser::parse_file(std::vector<lexeme>* lexemes) {
    input = lexemes;
    auto* target = new tree_node_list<tree_node*>();
    if (!target)
        return nullptr;

    if (!parse_grammatic_global(this, target)) {
        delete target;
        return nullptr;
    }

    return target;
}

bool parser::append_file(std::vector<lexeme>* lexemes, tree_node_list<tree_node*>* target) {
    input = lexemes;

    return parse_grammatic_global(this, target);
}

void parser::spit_lexeme() {
    position--;
}

lexeme* parser::next_lexeme() {
    return &(*input)[(int)position];
}

void parser::eat_lexeme() {
    lexeme* c = next_lexeme();
    assert(c->type);
    position++;
}

} // namespace bonk