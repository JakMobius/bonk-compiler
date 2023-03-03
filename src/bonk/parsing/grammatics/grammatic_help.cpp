
#include <fstream>
#include "../parser.hpp"

namespace bonk {

bool Parser::parse_help(TreeNodeList* target) {
    Lexeme* next = next_lexeme();
    if (!next->is(OperatorType::o_help)) {
        return false;
    }

    eat_lexeme();

    next = next_lexeme();
    if (next->type != LexemeType::l_identifier) {
        linked_compiler.error().at(next_lexeme()->start_position)
            << "expected library or file name to import";
        return false;
    }
    eat_lexeme();

    auto library_name = std::get<IdentifierLexeme>(next->data).identifier;

    std::string full_path = "/usr/local/lib/bonkScript/help/";
    full_path += library_name;
    full_path += ".bs";

    // Read the file
    std::ifstream ifstream(full_path);
    if (!ifstream.is_open()) {
        std::string library_name_str{library_name};
        linked_compiler.error().at(next_lexeme()->start_position)
            << "failed to open '" << library_name_str << "': " << strerror(errno);
        return false;
    }
    std::string source;
    ifstream.seekg(0, std::ios::end);
    source.reserve(ifstream.tellg());
    ifstream.seekg(0, std::ios::beg);
    source.assign((std::istreambuf_iterator<char>(ifstream)), std::istreambuf_iterator<char>());

//    if (linked_compiler.lexical_analyzer.file_already_compiled(full_path)) {
//        return true;
//    }

    std::vector<Lexeme> lexemes =
        linked_compiler.lexical_analyzer.parse_file(full_path.c_str(), source.c_str());

    if (linked_compiler.state) {
        return true;
    }

    Parser nested_parser{linked_compiler};

    if (!nested_parser.append_file(&lexemes, target)) {
        return false;
    }

    return true;
}

} // namespace bonk