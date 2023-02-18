

#include "grammatic_help.hpp"

namespace bonk {

bool parse_grammatic_help(parser* thou, tree_node_list<tree_node*>* target) {
    lexeme* next = thou->next_lexeme();
    if (next->type == BONK_LEXEME_KEYWORD && next->keyword_data.keyword_type == BONK_KEYWORD_HELP) {
        thou->eat_lexeme();

        next = thou->next_lexeme();
        if (next->type != BONK_LEXEME_IDENTIFIER) {
            thou->error("expected library or file name to import");
            return false;
        }
        thou->eat_lexeme();

        const char* library_storage = "/usr/local/lib/bonkScript/help/";
        const char* library_name = next->identifier_data.identifier->variable_name.c_str();
        const char* library_extension = ".bs";

        char* full_path = (char*)calloc(strlen(library_storage) + strlen(library_name) +
                                            strlen(library_extension) + 1,
                                        sizeof(char));

        int length = sprintf(full_path, "%s%s%s", library_storage, library_name, library_extension);

        full_path[length] = '\0';

        file_op_result read_result = FILE_OP_OK;
        const char* source = read_file(full_path, &read_result, nullptr);

        if (read_result == FILE_OP_NOT_ENOUGH_MEMORY) {
            free(full_path);
            free((void*)library_name);
            thou->linked_compiler->out_of_memory();
            return false;
        } else if (read_result != FILE_OP_OK) {
            free(full_path);
            thou->error("failed to open '%s': '%s'", library_name, strerror(errno));
            free((void*)library_name);
            return false;
        }

        free((void*)library_name);

        if (!thou->linked_compiler->lexical_analyzer->file_already_compiled(full_path)) {
            std::vector<lexeme> lexemes =
                thou->linked_compiler->lexical_analyzer->parse_file(full_path, source);

            if (!thou->linked_compiler->state) {
                parser* nested_parser = new parser(thou->linked_compiler);

                if (!nested_parser) {
                    free(full_path);
                    thou->linked_compiler->out_of_memory();
                    return false;
                }

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