#pragma once

namespace bonk {

enum class TreeNodeType {
    n_unset,
    n_program,
    n_help_statement,
    n_block_definition,
    n_hive_definition,
    n_variable_definition,
    n_parameter_list_definition,
    n_parameter_list_item,
    n_identifier,
    n_code_block,
    n_array_constant,
    n_number_constant,
    n_string_constant,
    n_bonk_statement,
    n_brek_statement,
    n_hive_access,
    n_loop_statement,
    n_primitive_type,
    n_binary_operation,
    n_unary_operation,
    n_many_type,
    n_call
};

struct ASTVisitor;
struct TreeNode;
struct TreeNodeProgram;
struct TreeNodeHelp;
struct TreeNodeIdentifier;
struct TreeNodeBlockDefinition;
struct TreeNodeHiveDefinition;
struct TreeNodeVariableDefinition;
struct TreeNodeParameterListDefinition;
struct TreeNodeParameterList;
struct TreeNodeParameterListItem;
struct TreeNodeCodeBlock;
struct TreeNodeArrayConstant;
struct TreeNodeNumberConstant;
struct TreeNodeStringConstant;
struct TreeNodeBinaryOperation;
struct TreeNodeUnaryOperation;
struct TreeNodePrimitiveType;
struct TreeNodeManyType;
struct TreeNodeHiveAccess;
struct TreeNodeBonkStatement;
struct TreeNodeBrekStatement;
struct TreeNodeLoopStatement;

enum class PrimitiveType { t_unset, t_buul, t_shrt, t_nubr, t_long, t_flot, t_dabl, t_strg };
extern const char* BONK_PRIMITIVE_TYPE_NAMES[];

} // namespace bonk

#include <list>
#include <string>
#include "bonk/parsing/lexic/lexer.hpp"
#include "bonk/parsing/parser_position.hpp"

namespace bonk {

struct TreeNode {
    TreeNodeType type{};
    ParserPosition source_position{};

    TreeNode() = default;
    virtual ~TreeNode() = default;

    virtual void accept(ASTVisitor* visitor) = 0;
};

struct TreeNodeProgram : TreeNode {
    std::list<std::unique_ptr<TreeNodeHelp>> help_statements{};
    std::list<std::unique_ptr<TreeNode>> body{};

    TreeNodeProgram() {
        type = TreeNodeType::n_program;
    }

    void accept(ASTVisitor* visitor) override;
};

struct TreeNodeHelp : TreeNode {
    std::unique_ptr<TreeNodeStringConstant> string{};

    TreeNodeHelp() {
        type = TreeNodeType::n_help_statement;
    }

    void accept(ASTVisitor* visitor) override;
};

struct TreeNodeIdentifier : TreeNode {
    std::string_view identifier_text{};

    TreeNodeIdentifier() {
        type = TreeNodeType::n_identifier;
    }

    void accept(ASTVisitor* visitor) override;
};

struct TreeNodeBlockDefinition : TreeNode {
    std::unique_ptr<TreeNodeIdentifier> block_name{};
    std::unique_ptr<TreeNodeParameterListDefinition> block_parameters{};
    std::unique_ptr<TreeNodeCodeBlock> body{};
    std::unique_ptr<TreeNode> return_type{};

    TreeNodeBlockDefinition() {
        type = TreeNodeType::n_block_definition;
    }

    void accept(ASTVisitor* visitor) override;
};

struct TreeNodeHiveDefinition : TreeNode {
    std::unique_ptr<TreeNodeIdentifier> hive_name{};
    std::list<std::unique_ptr<TreeNode>> body{};

    TreeNodeHiveDefinition() {
        type = TreeNodeType::n_hive_definition;
    }

    void accept(ASTVisitor* visitor) override;
};

struct TreeNodeVariableDefinition : TreeNode {
    std::unique_ptr<TreeNodeIdentifier> variable_name{};
    std::unique_ptr<TreeNode> variable_value{};
    std::unique_ptr<TreeNode> variable_type{};

    TreeNodeVariableDefinition() {
        type = TreeNodeType::n_variable_definition;
    }

    void accept(ASTVisitor* visitor) override;
};

struct TreeNodeParameterListDefinition : TreeNode {
    std::list<std::unique_ptr<TreeNodeVariableDefinition>> parameters{};

    TreeNodeParameterListDefinition() {
        type = TreeNodeType::n_parameter_list_definition;
    }

    void accept(ASTVisitor* visitor) override;
};

struct TreeNodeParameterList : TreeNode {
    std::list<std::unique_ptr<TreeNodeParameterListItem>> parameters{};

    TreeNodeParameterList() {
        type = TreeNodeType::n_parameter_list_definition;
    }

    void accept(ASTVisitor* visitor) override;
};

struct TreeNodeParameterListItem : TreeNode {
    std::unique_ptr<TreeNodeIdentifier> parameter_name{};
    std::unique_ptr<TreeNode> parameter_value{};

    TreeNodeParameterListItem() {
        type = TreeNodeType::n_parameter_list_item;
    }

    void accept(ASTVisitor* visitor) override;
};

struct TreeNodeCodeBlock : TreeNode {
    std::list<std::unique_ptr<TreeNode>> body{};

    TreeNodeCodeBlock() {
        type = TreeNodeType::n_code_block;
    }

    void accept(ASTVisitor* visitor) override;
};

struct TreeNodeBonkStatement : TreeNode {
    std::unique_ptr<TreeNode> expression{};

    TreeNodeBonkStatement() {
        type = TreeNodeType::n_bonk_statement;
    }

    void accept(ASTVisitor* visitor) override;
};

struct TreeNodeBrekStatement : TreeNode {
    TreeNodeBrekStatement() {
        type = TreeNodeType::n_brek_statement;
    }

    void accept(ASTVisitor* visitor) override;
};

struct TreeNodeArrayConstant : TreeNode {
    std::list<std::unique_ptr<TreeNode>> elements{};

    TreeNodeArrayConstant() {
        type = TreeNodeType::n_array_constant;
    }

    void accept(ASTVisitor* visitor) override;
};

struct TreeNodeNumberConstant : TreeNode {
    NumberConstantContents contents;

    TreeNodeNumberConstant() {
        type = TreeNodeType::n_number_constant;
    }

    void accept(ASTVisitor* visitor) override;
};

struct TreeNodeStringConstant : TreeNode {
    std::string_view string_value{};

    TreeNodeStringConstant() {
        type = TreeNodeType::n_string_constant;
    }

    void accept(ASTVisitor* visitor) override;
};

struct TreeNodeHiveAccess : TreeNode {
    std::unique_ptr<TreeNode> hive{};
    std::unique_ptr<TreeNodeIdentifier> field{};

    TreeNodeHiveAccess() {
        type = TreeNodeType::n_hive_access;
    }

    void accept(ASTVisitor* visitor) override;
};

struct TreeNodeLoopStatement : TreeNode {
    std::unique_ptr<TreeNodeParameterListDefinition> loop_parameters;
    std::unique_ptr<TreeNodeCodeBlock> body;

    TreeNodeLoopStatement() {
        type = TreeNodeType::n_loop_statement;
    }

    void accept(ASTVisitor* visitor) override;
};

struct TreeNodeManyType : TreeNode {
    std::unique_ptr<TreeNode> parameter;

    TreeNodeManyType() {
        type = TreeNodeType::n_many_type;
    }

    void accept(ASTVisitor* visitor) override;
};

struct TreeNodePrimitiveType : TreeNode {
    PrimitiveType primitive_type{};

    TreeNodePrimitiveType() {
        type = TreeNodeType::n_primitive_type;
    }

    void accept(ASTVisitor* visitor) override;
};

struct TreeNodeBinaryOperation : TreeNode {
    std::unique_ptr<TreeNode> left{};
    std::unique_ptr<TreeNode> right{};
    OperatorType operator_type{};

    TreeNodeBinaryOperation() {
        type = TreeNodeType::n_binary_operation;
    }

    void accept(ASTVisitor* visitor) override;
};

struct TreeNodeUnaryOperation : TreeNode {
    std::unique_ptr<TreeNode> operand{};
    OperatorType operator_type{};

    TreeNodeUnaryOperation() {
        type = TreeNodeType::n_unary_operation;
    }

    void accept(ASTVisitor* visitor) override;
};

struct TreeNodeCall : TreeNode {
    std::unique_ptr<TreeNode> callee{};
    std::unique_ptr<TreeNodeParameterList> arguments{};

    TreeNodeCall() {
        type = TreeNodeType::n_call;
    }

    void accept(ASTVisitor* visitor) override;
};

} // namespace bonk