
#include "x86_backend.hpp"
#include "x86_backend_context.hpp"

namespace bonk::x86_backend {

bool Backend::compile_ast(std::unique_ptr<TreeNode> ast, const OutputStream& output) {

    macho::MachoFile file;
    BackendContext context{&linked_compiler, &file};

//    context.compile_program(ast);

    /*
     * TODO: get rid of global compiler state in
     * order to make it possible to do parallel
     * compilation
     */

    if (linked_compiler.state) {
        linked_compiler.state = BONK_COMPILER_OK;
        return false;
    }

    file.flush(output);

    return true;
}

} // namespace bonk::x86_backend