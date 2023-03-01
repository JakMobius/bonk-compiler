
#include "compiler.hpp"

namespace bonk {

void Compiler::out_of_memory() {
    fatal_error("Compiler ran out of memory");
}

Compiler::Compiler(CompilerConfig* the_config) {
    config = the_config;

    // TODO: better naming
    lexical_analyzer = new LexicalAnalyzer(this);
    parser = new Parser(this);
}

Compiler::~Compiler() {
    delete parser;
    delete lexical_analyzer;

    config = nullptr;
    lexical_analyzer = nullptr;
    parser = nullptr;
}

bool Compiler::compile_ast(TreeNodeList* ast, FILE* target) {
    if (!config->compile_backend) {
        fatal_error("backend has not been specified");
        return false;
    }

    config->compile_backend->compile_ast(this, ast, target);

    return false;
}

} // namespace bonk