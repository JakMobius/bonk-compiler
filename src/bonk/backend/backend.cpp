
#include "backend.hpp"

namespace bonk {

bool backend::compile_ast(compiler* linked_compiler, tree_node_list<tree_node*>* ast,
                          FILE* target) {
    printf("abstract method compile_ast was not implemented");
    abort();
}

}
