
#include "backend.hpp"

namespace bonk {

bool Backend::compile_ast(Compiler* linked_compiler, TreeNodeList<TreeNode*>* ast,
                          FILE* target) {
    printf("abstract method compile_ast was not implemented");
    abort();
}

}
