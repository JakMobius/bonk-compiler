#pragma once

namespace bonk {

enum class TreeNodeType {
    n_number,
    n_identifier,
    n_operator,
    n_list,
    n_block_definition,
    n_var_definition,
    n_call,
    n_call_parameter,
    n_check,
    n_cycle,
    n_unset
};

struct TreeNodeBlockDefinition;
struct TreeNodeCall;
struct TreeNodeCallParameter;
struct TreeNodeCheck;
struct TreeNodeCycle;
struct TreeNodeIdentifier;
struct TreeNodeList;
struct TreeNodeNumber;
struct TreeNodeOperator;
struct TreeNodeVariableDefinition;
struct ASTVisitor;

}

#include <list>
#include <string>
#include "bonk/parsing/lexic/lexical_analyzer.hpp"
#include "bonk/parsing/parser_position.hpp"

namespace bonk {

struct TreeNode {
    TreeNodeType type = TreeNodeType::n_unset;
    ParserPosition source_position {};

    TreeNode() = default;
    virtual ~TreeNode() = default;

    virtual void accept(ASTVisitor* visitor) {}
};

struct TreeNodeBlockDefinition : TreeNode {

    std::unique_ptr<TreeNodeIdentifier> block_name;
    std::unique_ptr<TreeNodeList> body;

    TreeNodeBlockDefinition();

    void accept(ASTVisitor* visitor) override;
};

struct TreeNodeCall : TreeNode {

    std::unique_ptr<TreeNodeIdentifier> call_function;
    std::unique_ptr<TreeNodeList> call_parameters;

    TreeNodeCall();

    void accept(ASTVisitor* visitor) override;
};

struct TreeNodeCallParameter : TreeNode {

    std::unique_ptr<TreeNodeIdentifier> parameter_name;
    std::unique_ptr<TreeNode> parameter_value;

    TreeNodeCallParameter();

    void accept(ASTVisitor* visitor) override;
};

struct TreeNodeCheck : TreeNode {

    std::unique_ptr<TreeNode> condition;
    std::unique_ptr<TreeNodeList> check_body;
    std::unique_ptr<TreeNodeList> or_body;

    TreeNodeCheck();

    void accept(ASTVisitor* visitor) override;
};

struct TreeNodeCycle : TreeNode {
    std::unique_ptr<TreeNodeList> body;

    TreeNodeCycle();

    void accept(ASTVisitor* visitor) override;
};

struct TreeNodeIdentifier : TreeNode {
    std::string_view variable_name;

    explicit TreeNodeIdentifier();

    void accept(ASTVisitor* visitor) override;
};

struct TreeNodeList : TreeNode {

    std::list<std::unique_ptr<TreeNode>> list;

    TreeNodeList();

    void accept(ASTVisitor* visitor) override;
};

struct TreeNodeNumber : TreeNode {
    long long integer_value = 0;
    long double float_value = 0;

    TreeNodeNumber();

    void accept(ASTVisitor* visitor) override;
};

struct TreeNodeOperator : TreeNode {
    OperatorType oper_type = OperatorType::o_invalid;
    std::unique_ptr<TreeNode> left;
    std::unique_ptr<TreeNode> right;

    explicit TreeNodeOperator();

    void accept(ASTVisitor* visitor) override;
};

struct TreeNodeVariableDefinition : TreeNode {
    bool is_contextual = false;
    std::unique_ptr<TreeNodeIdentifier> variable_name;
    std::unique_ptr<TreeNode> variable_value;

    TreeNodeVariableDefinition();

    void accept(ASTVisitor* visitor) override;
};

}