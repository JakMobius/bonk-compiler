
#include "ast.hpp"
#include "ast_visitor.hpp"

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

TreeNodeBlockDefinition::TreeNodeBlockDefinition() : TreeNode() {
    type = TREE_NODE_TYPE_BLOCK_DEFINITION;
}

TreeNodeBlockDefinition::~TreeNodeBlockDefinition() {
    delete body;
    delete block_name;

    body = nullptr;
    block_name = nullptr;
}

void TreeNodeBlockDefinition::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

TreeNodeCall::TreeNodeCall(TreeNodeIdentifier* function, TreeNodeList* parameters) : TreeNode() {
    call_function = function;
    call_parameters = parameters;
    type = TREE_NODE_TYPE_CALL;
}

TreeNodeCall::~TreeNodeCall() {
    delete call_function;
    delete call_parameters;
    call_function = nullptr;
    call_parameters = nullptr;
}

void TreeNodeCall::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

TreeNodeCallParameter::TreeNodeCallParameter(TreeNodeIdentifier* name, TreeNode* value)
    : TreeNode() {
    parameter_name = name;
    parameter_value = value;
    type = TREE_NODE_TYPE_CALL_PARAMETER;
}

TreeNodeCallParameter::~TreeNodeCallParameter() {
    delete parameter_name;
    delete parameter_value;

    parameter_name = nullptr;
    parameter_value = nullptr;
}

void TreeNodeCallParameter::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

TreeNodeCheck::TreeNodeCheck() : TreeNode() {
    type = TREE_NODE_TYPE_CHECK;
}

TreeNodeCheck::~TreeNodeCheck() {
    delete condition;
    delete check_body;
    delete or_body;

    condition = nullptr;
    check_body = nullptr;
    or_body = nullptr;
}

void TreeNodeCheck::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

TreeNodeCycle::TreeNodeCycle() : TreeNode() {
    type = TREE_NODE_TYPE_CYCLE;
}

void TreeNodeCycle::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

TreeNodeIdentifier::TreeNodeIdentifier(const std::string& name) : TreeNode() {
    variable_name = name;
    type = TREE_NODE_TYPE_IDENTIFIER;
}

bool TreeNodeIdentifier::contents_equal(TreeNodeIdentifier* other) {
    return variable_name == other->variable_name;
}

void TreeNodeIdentifier::print(FILE* file) const {
    fprintf(file, "%s", variable_name.c_str());
}

void TreeNodeIdentifier::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

TreeNodeList::TreeNodeList() {
    type = TREE_NODE_TYPE_LIST;
}

TreeNodeList::~TreeNodeList() {
    for (auto& i : list) {
        delete i;
    }
}

void TreeNodeList::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

TreeNodeNumber::TreeNodeNumber(long double the_float_value, long long the_integer_value)
    : TreeNode() {
    float_value = the_float_value;
    integer_value = the_integer_value;
    type = TREE_NODE_TYPE_NUMBER;
}

void TreeNodeNumber::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

TreeNodeOperator::TreeNodeOperator(OperatorType oper) {
    oper_type = oper;
    type = TREE_NODE_TYPE_OPERATOR;
}

void TreeNodeOperator::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

TreeNodeVariableDefinition::TreeNodeVariableDefinition(bool contextual,
                                                       TreeNodeIdentifier* identifier) {
    variable_name = identifier;
    is_contextual = contextual;
    type = TREE_NODE_TYPE_VAR_DEFINITION;
}

TreeNodeVariableDefinition::~TreeNodeVariableDefinition() {
    delete variable_name;
    delete variable_value;

    variable_name = nullptr;
    variable_value = nullptr;
}
void TreeNodeVariableDefinition::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

} // namespace bonk
