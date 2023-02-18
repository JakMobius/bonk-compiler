
#include "ede_backend.hpp"

namespace bonk::ede_backend {

bool backend::compile_ast(compiler* linked_compiler, tree_node_list<bonk::tree_node*>* ast,
                          FILE* target) {

    backend_context context(linked_compiler, target);
    context.compile_program(ast);

    if (linked_compiler->state) {
        linked_compiler->state = BONK_COMPILER_OK;
        return false;
    }

    return true;
}

} // namespace bonk::ede_backend