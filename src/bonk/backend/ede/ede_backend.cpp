
#include "ede_backend.hpp"

namespace bonk::ede_backend {

bool Backend::compile_ast(Compiler* linked_compiler, TreeNodeList* ast,
                          FILE* target) {

    BackendContext context(linked_compiler, target);
    context.compile_program(ast);

    if (linked_compiler->state) {
        linked_compiler->state = BONK_COMPILER_OK;
        return false;
    }

    return true;
}

} // namespace bonk::ede_backend