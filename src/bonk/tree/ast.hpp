#pragma once

namespace bonk {

enum class TreeNodeType : char {
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
    n_call,
    n_cast,
    n_null
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
struct TreeNodeCall;

enum class TrivialTypeKind : char {
    t_unset,
    t_buul,
    t_shrt,
    t_nubr,
    t_long,
    t_flot,
    t_dabl,
    t_strg,
    t_never,
    t_nothing
};
extern const char* BONK_TRIVIAL_TYPE_KIND_NAMES[];

} // namespace bonk

#include <memory>
#include <list>
#include <string>
#include "bonk/parsing/lexic/lexer.hpp"
#include "bonk/parsing/parser_position.hpp"
#include "utils/buffer.hpp"

namespace bonk {

struct TreeNode {
    TreeNodeType type{};
    ParserPosition source_position{};

    TreeNode() = default;
    virtual ~TreeNode() = default;

    virtual void accept(ASTVisitor* visitor) = 0;

    template <typename T> void fields(T&& callback) {
        callback(type, "type");
        callback(source_position, "source_position");
    };

    static std::unique_ptr<TreeNode> create(TreeNodeType type);
};

struct TreeNodeProgram : TreeNode {
    std::list<std::unique_ptr<TreeNodeHelp>> help_statements{};
    std::list<std::unique_ptr<TreeNode>> body{};

    TreeNodeProgram() {
        type = TreeNodeType::n_program;
    }

    void accept(ASTVisitor* visitor) override;

    template <typename T> void fields(T&& callback) {
        TreeNode::fields(callback);
        callback(help_statements, "help_statements");
        callback(body, "body");
    }
};

struct TreeNodeHelp : TreeNode {
    std::unique_ptr<TreeNodeStringConstant> string{};

    TreeNodeHelp() {
        type = TreeNodeType::n_help_statement;
    }

    void accept(ASTVisitor* visitor) override;

    template <typename T> void fields(T&& callback) {
        TreeNode::fields(callback);
        callback(string, "string");
    }
};

struct TreeNodeIdentifier : TreeNode {
    std::string_view identifier_text{};

    TreeNodeIdentifier() {
        type = TreeNodeType::n_identifier;
    }

    void accept(ASTVisitor* visitor) override;

    template <typename T> void fields(T&& callback) {
        TreeNode::fields(callback);
        callback(identifier_text, "identifier_text");
    }
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

    template <typename T> void fields(T&& callback) {
        TreeNode::fields(callback);
        callback(block_name, "block_name");
        callback(block_parameters, "block_parameters");
        callback(body, "body");
        callback(return_type, "return_type");
    }
};

struct TreeNodeHiveDefinition : TreeNode {
    std::unique_ptr<TreeNodeIdentifier> hive_name{};
    std::list<std::unique_ptr<TreeNode>> body{};

    TreeNodeHiveDefinition() {
        type = TreeNodeType::n_hive_definition;
    }

    void accept(ASTVisitor* visitor) override;

    template <typename T> void fields(T&& callback) {
        TreeNode::fields(callback);
        callback(hive_name, "hive_name");
        callback(body, "body");
    }
};

struct TreeNodeVariableDefinition : TreeNode {
    std::unique_ptr<TreeNodeIdentifier> variable_name{};
    std::unique_ptr<TreeNode> variable_value{};
    std::unique_ptr<TreeNode> variable_type{};

    TreeNodeVariableDefinition() {
        type = TreeNodeType::n_variable_definition;
    }

    void accept(ASTVisitor* visitor) override;

    template <typename T> void fields(T&& callback) {
        TreeNode::fields(callback);
        callback(variable_name, "variable_name");
        callback(variable_value, "variable_value");
        callback(variable_type, "variable_type");
    }
};

struct TreeNodeParameterListDefinition : TreeNode {
    std::list<std::unique_ptr<TreeNodeVariableDefinition>> parameters{};

    TreeNodeParameterListDefinition() {
        type = TreeNodeType::n_parameter_list_definition;
    }

    void accept(ASTVisitor* visitor) override;

    template <typename T> void fields(T&& callback) {
        TreeNode::fields(callback);
        callback(parameters, "parameters");
    }
};

struct TreeNodeParameterList : TreeNode {
    std::list<std::unique_ptr<TreeNodeParameterListItem>> parameters{};

    TreeNodeParameterList() {
        type = TreeNodeType::n_parameter_list_definition;
    }

    void accept(ASTVisitor* visitor) override;

    template <typename T> void fields(T&& callback) {
        TreeNode::fields(callback);
        callback(parameters, "parameters");
    }
};

struct TreeNodeParameterListItem : TreeNode {
    std::unique_ptr<TreeNodeIdentifier> parameter_name{};
    std::unique_ptr<TreeNode> parameter_value{};

    TreeNodeParameterListItem() {
        type = TreeNodeType::n_parameter_list_item;
    }

    void accept(ASTVisitor* visitor) override;

    template <typename T> void fields(T&& callback) {
        TreeNode::fields(callback);
        callback(parameter_name, "parameter_name");
        callback(parameter_value, "parameter_value");
    }
};

struct TreeNodeCodeBlock : TreeNode {
    std::list<std::unique_ptr<TreeNode>> body{};

    TreeNodeCodeBlock() {
        type = TreeNodeType::n_code_block;
    }

    void accept(ASTVisitor* visitor) override;

    template <typename T> void fields(T&& callback) {
        TreeNode::fields(callback);
        callback(body, "body");
    }
};

struct TreeNodeBonkStatement : TreeNode {
    std::unique_ptr<TreeNode> expression{};

    TreeNodeBonkStatement() {
        type = TreeNodeType::n_bonk_statement;
    }

    void accept(ASTVisitor* visitor) override;

    template <typename T> void fields(T&& callback) {
        TreeNode::fields(callback);
        callback(expression, "expression");
    }
};

struct TreeNodeBrekStatement : TreeNode {
    TreeNodeBrekStatement() {
        type = TreeNodeType::n_brek_statement;
    }

    void accept(ASTVisitor* visitor) override;

    template <typename T> void fields(T&& callback) {
        TreeNode::fields(callback);
    }
};

struct TreeNodeArrayConstant : TreeNode {
    std::list<std::unique_ptr<TreeNode>> elements{};

    TreeNodeArrayConstant() {
        type = TreeNodeType::n_array_constant;
    }

    void accept(ASTVisitor* visitor) override;

    template <typename T> void fields(T&& callback) {
        TreeNode::fields(callback);
        callback(elements, "elements");
    }
};

struct TreeNodeNumberConstant : TreeNode {
    NumberConstantContents contents;

    TreeNodeNumberConstant() {
        type = TreeNodeType::n_number_constant;
    }

    void accept(ASTVisitor* visitor) override;

    template <typename T> void fields(T&& callback) {
        TreeNode::fields(callback);
        callback(contents, "contents");
    }
};

struct TreeNodeStringConstant : TreeNode {
    std::string string_value{};

    TreeNodeStringConstant() {
        type = TreeNodeType::n_string_constant;
    }

    void accept(ASTVisitor* visitor) override;

    template <typename T> void fields(T&& callback) {
        TreeNode::fields(callback);
        callback(string_value, "string_value");
    }
};

struct TreeNodeHiveAccess : TreeNode {
    std::unique_ptr<TreeNode> hive{};
    std::unique_ptr<TreeNodeIdentifier> field{};

    TreeNodeHiveAccess() {
        type = TreeNodeType::n_hive_access;
    }

    void accept(ASTVisitor* visitor) override;

    template <typename T> void fields(T&& callback) {
        TreeNode::fields(callback);
        callback(hive, "hive");
        callback(field, "field");
    }
};

struct TreeNodeLoopStatement : TreeNode {
    std::unique_ptr<TreeNodeParameterListDefinition> loop_parameters;
    std::unique_ptr<TreeNodeCodeBlock> body;

    TreeNodeLoopStatement() {
        type = TreeNodeType::n_loop_statement;
    }

    void accept(ASTVisitor* visitor) override;

    template <typename T> void fields(T&& callback) {
        TreeNode::fields(callback);
        callback(loop_parameters, "loop_parameters");
        callback(body, "body");
    }
};

struct TreeNodeNull : TreeNode {
    TreeNodeNull() {
        type = TreeNodeType::n_null;
    }

    void accept(ASTVisitor* visitor) override;

    template <typename T> void fields(T&& callback) {
        TreeNode::fields(callback);
    }
};

struct TreeNodeManyType : TreeNode {
    std::unique_ptr<TreeNode> parameter;

    TreeNodeManyType() {
        type = TreeNodeType::n_many_type;
    }

    void accept(ASTVisitor* visitor) override;

    template <typename T> void fields(T&& callback) {
        TreeNode::fields(callback);
    }
};

struct TreeNodePrimitiveType : TreeNode {
    TrivialTypeKind primitive_type{};

    TreeNodePrimitiveType() {
        type = TreeNodeType::n_primitive_type;
    }

    void accept(ASTVisitor* visitor) override;

    template <typename T> void fields(T&& callback) {
        TreeNode::fields(callback);
        callback(primitive_type, "primitive_type");
    }
};

struct TreeNodeBinaryOperation : TreeNode {
    std::unique_ptr<TreeNode> left{};
    std::unique_ptr<TreeNode> right{};
    OperatorType operator_type{};

    TreeNodeBinaryOperation() {
        type = TreeNodeType::n_binary_operation;
    }

    void accept(ASTVisitor* visitor) override;

    template <typename T> void fields(T&& callback) {
        TreeNode::fields(callback);
        callback(left, "left");
        callback(right, "right");
        callback(operator_type, "operator_type");
    }
};

struct TreeNodeUnaryOperation : TreeNode {
    std::unique_ptr<TreeNode> operand{};
    OperatorType operator_type{};

    TreeNodeUnaryOperation() {
        type = TreeNodeType::n_unary_operation;
    }

    void accept(ASTVisitor* visitor) override;

    template <typename T> void fields(T&& callback) {
        TreeNode::fields(callback);
        callback(operand, "operand");
        callback(operator_type, "operator_type");
    }
};

struct TreeNodeCall : TreeNode {
    std::unique_ptr<TreeNode> callee{};
    std::unique_ptr<TreeNodeParameterList> arguments{};

    TreeNodeCall() {
        type = TreeNodeType::n_call;
    }

    void accept(ASTVisitor* visitor) override;

    template <typename T> void fields(T&& callback) {
        TreeNode::fields(callback);
        callback(callee, "callee");
        callback(arguments, "arguments");
    }
};

struct TreeNodeCast : TreeNode {
    std::unique_ptr<TreeNode> operand{};
    std::unique_ptr<TreeNode> target_type{};

    TreeNodeCast() {
        type = TreeNodeType::n_cast;
    }

    void accept(ASTVisitor* visitor) override;

    template <typename T> void fields(T&& callback) {
        TreeNode::fields(callback);
        callback(operand, "operand");
        callback(target_type, "target_type");
    }
};

struct AST {
    std::unique_ptr<TreeNodeProgram> root{};
    bonk::Buffer buffer{};

    AST() = default;
    AST(const AST&) = delete;
    AST& operator=(const AST&) = delete;
    AST(AST&&) = default;
    AST& operator=(AST&&) = default;
};

} // namespace bonk