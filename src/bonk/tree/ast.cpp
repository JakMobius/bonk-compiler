
#include <cassert>
#include "ast.hpp"
#include "ast_visitor.hpp"

namespace bonk {

const char* BONK_TRIVIAL_TYPE_KIND_NAMES[] = {
    "unset",
    "buul",
    "shrt",
    "nubr",
    "long",
    "flot",
    "dabl",
    "strg",
    "never",
    "nothing"
};

void TreeNodeProgram::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}
void TreeNodeHelp::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}
void TreeNodeIdentifier::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}
void TreeNodeBlockDefinition::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}
void TreeNodeHiveDefinition::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}
void TreeNodeVariableDefinition::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}
void TreeNodeParameterListDefinition::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}
void TreeNodeParameterList::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}
void TreeNodeParameterListItem::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}
void TreeNodeCodeBlock::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}
void TreeNodeBonkStatement::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}
void TreeNodeBrekStatement::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}
void TreeNodeArrayConstant::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}
void TreeNodeNumberConstant::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}
void TreeNodeStringConstant::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}
void TreeNodeHiveAccess::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}
void TreeNodeLoopStatement::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}
void TreeNodeManyType::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}
void TreeNodePrimitiveType::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}
void TreeNodeBinaryOperation::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}
void TreeNodeUnaryOperation::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}
void TreeNodeCall::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}
void TreeNodeCast::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}
void TreeNodeNull::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

std::unique_ptr<TreeNode> TreeNode::create(TreeNodeType type) {
    switch(type) {

    case TreeNodeType::n_unset:
        return nullptr;
    case TreeNodeType::n_program:
        return std::make_unique<TreeNodeProgram>();
    case TreeNodeType::n_help_statement:
        return std::make_unique<TreeNodeHelp>();
    case TreeNodeType::n_block_definition:
        return std::make_unique<TreeNodeBlockDefinition>();
    case TreeNodeType::n_hive_definition:
        return std::make_unique<TreeNodeHiveDefinition>();
    case TreeNodeType::n_variable_definition:
        return std::make_unique<TreeNodeVariableDefinition>();
    case TreeNodeType::n_parameter_list_definition:
        return std::make_unique<TreeNodeParameterListDefinition>();
    case TreeNodeType::n_parameter_list_item:
        return std::make_unique<TreeNodeParameterListItem>();
    case TreeNodeType::n_identifier:
        return std::make_unique<TreeNodeIdentifier>();
    case TreeNodeType::n_code_block:
        return std::make_unique<TreeNodeCodeBlock>();
    case TreeNodeType::n_array_constant:
        return std::make_unique<TreeNodeArrayConstant>();
    case TreeNodeType::n_number_constant:
        return std::make_unique<TreeNodeNumberConstant>();
    case TreeNodeType::n_string_constant:
        return std::make_unique<TreeNodeStringConstant>();
    case TreeNodeType::n_bonk_statement:
        return std::make_unique<TreeNodeBonkStatement>();
    case TreeNodeType::n_brek_statement:
        return std::make_unique<TreeNodeBrekStatement>();
    case TreeNodeType::n_hive_access:
        return std::make_unique<TreeNodeHiveAccess>();
    case TreeNodeType::n_loop_statement:
        return std::make_unique<TreeNodeLoopStatement>();
    case TreeNodeType::n_primitive_type:
        return std::make_unique<TreeNodePrimitiveType>();
    case TreeNodeType::n_binary_operation:
        return std::make_unique<TreeNodeBinaryOperation>();
    case TreeNodeType::n_unary_operation:
        return std::make_unique<TreeNodeUnaryOperation>();
    case TreeNodeType::n_many_type:
        return std::make_unique<TreeNodeManyType>();
    case TreeNodeType::n_call:
        return std::make_unique<TreeNodeCall>();
    case TreeNodeType::n_cast:
        return std::make_unique<TreeNodeCast>();
    case TreeNodeType::n_null:
        return std::make_unique<TreeNodeNull>();
    default:
        assert(!"Unknown node type");
    }
}
} // namespace bonk
