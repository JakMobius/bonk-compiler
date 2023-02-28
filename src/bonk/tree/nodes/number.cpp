
#include "number.hpp"
#include "bonk/tree/ast_visitor.hpp"

namespace bonk {

TreeNodeNumber::TreeNodeNumber(long double the_float_value, long long the_integer_value)
    : TreeNode() {
    float_value = the_float_value;
    integer_value = the_integer_value;
    type = TREE_NODE_TYPE_NUMBER;
}

void TreeNodeNumber::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

} // namespace bonk