
#include "operator.hpp"
#include "bonk/tree/ast_visitor.hpp"

namespace bonk {

const char* OPERATOR_TYPE_NAMES[] = {"PLUS",
                                     "MINUS",
                                     "MULTIPLY",
                                     "DIVIDE",
                                     "ASSIGNMENT",
                                     "EQUALS",
                                     "NOT_EQUAL",
                                     "LESS_THAN",
                                     "GREATER_THAN",
                                     "LESS_OR_EQUAL_THAN",
                                     "GREATER_OR_EQUAL_THAN",
                                     "CYCLE",
                                     "CHECK",
                                     "PRINT",
                                     "BONK",
                                     "BREK",
                                     "BAMS",
                                     "AND",
                                     "OR",
                                     "REBONK",
                                     "INVALID"};

TreeNodeOperator::TreeNodeOperator(OperatorType oper) {
    oper_type = oper;
    type = TREE_NODE_TYPE_OPERATOR;
}

void TreeNodeOperator::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

} // namespace bonk