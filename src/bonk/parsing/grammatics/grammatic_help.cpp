
#include "../parser.hpp"
#include "utils/file_io.hpp"

namespace bonk {

bool Parser::parse_help(TreeNodeList* target) {
    Lexeme* next = next_lexeme();
    if (next->type == BONK_LEXEME_KEYWORD && next->keyword_data.keyword_type == BONK_KEYWORD_HELP) {
        eat_lexeme();

        next = next_lexeme();
        if (next->type != BONK_LEXEME_IDENTIFIER) {
            error("expected library or file name to import");
            return false;
        }
        eat_lexeme();

        const char* library_storage = "/usr/local/lib/bonkScript/help/";
        const char* library_name = next->identifier_data.identifier->variable_name.c_str();
        const char* library_extension = ".bs";

        char* full_path = (char*)calloc(strlen(library_storage) + strlen(library_name) +
                                            strlen(library_extension) + 1,
                                        sizeof(char));

        int length = sprintf(full_path, "%s%s%s", library_storage, library_name, library_extension);

        full_path[length] = '\0';

        FileOpResult read_result = FILE_OP_OK;
        const char* source = read_file(full_path, &read_result, nullptr);

        if (read_result == FILE_OP_NOT_ENOUGH_MEMORY) {
            free(full_path);
            free((void*)library_name);
            linked_compiler->out_of_memory();
            return false;
        } else if (read_result != FILE_OP_OK) {
            free(full_path);
            error("failed to open '%s': '%s'", library_name, strerror(errno));
            free((void*)library_name);
            return false;
        }

        free((void*)library_name);

        if (!linked_compiler->lexical_analyzer->file_already_compiled(full_path)) {
            std::vector<Lexeme> lexemes =
                linked_compiler->lexical_analyzer->parse_file(full_path, source);

            if (!linked_compiler->state) {
                auto* nested_parser = new Parser(linked_compiler);

                if (!nested_parser->append_file(&lexemes, target)) {
                    return false;
                }

                delete nested_parser;
            }
        }

        free((void*)source);

        return true;
    }

    return false;
}

} // namespace bonk