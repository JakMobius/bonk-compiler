
#include "backend.hpp"

namespace bonk {

bool Backend::compile_ast(std::unique_ptr<TreeNode> ast, const OutputStream& target) {
    printf("abstract method compile_ast was not implemented");
    abort();
}

} // namespace bonk
