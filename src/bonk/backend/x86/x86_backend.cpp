
#include "x86_backend.hpp"
#include "x86_backend_context.hpp"

namespace bonk::x86_backend {

bool backend::compile_ast(compiler* linked_compiler, tree_node_list<bonk::tree_node*>* ast,
                          FILE* target) {

    macho::macho_file* file = new macho::macho_file();
    backend_context context{linked_compiler, file};

    context.compile_program(ast);

    /*
     * TODO: get rid of global compiler state in
     * order to make it possible to do parallel
     * compilation
     */

    if (linked_compiler->state) {
        linked_compiler->state = BONK_COMPILER_OK;
        return false;
    }

    file->flush(target);
    delete file;

    return true;
}

} // namespace bonk::x86_backend