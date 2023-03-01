#pragma once

namespace bonk {

enum TreeNodeType {
    TREE_NODE_TYPE_NUMBER,
    TREE_NODE_TYPE_IDENTIFIER,
    TREE_NODE_TYPE_OPERATOR,
    TREE_NODE_TYPE_LIST,
    TREE_NODE_TYPE_BLOCK_DEFINITION,
    TREE_NODE_TYPE_VAR_DEFINITION,
    TREE_NODE_TYPE_CALL,
    TREE_NODE_TYPE_CALL_PARAMETER,
    TREE_NODE_TYPE_CHECK,
    TREE_NODE_TYPE_CYCLE,
    TREE_NODE_TYPE_INVALID
};

enum OperatorType {
    BONK_OPERATOR_PLUS,
    BONK_OPERATOR_MINUS,
    BONK_OPERATOR_MULTIPLY,
    BONK_OPERATOR_DIVIDE,
    BONK_OPERATOR_ASSIGNMENT,
    BONK_OPERATOR_EQUALS,
    BONK_OPERATOR_NOT_EQUAL,
    BONK_OPERATOR_LESS_THAN,
    BONK_OPERATOR_GREATER_THAN,
    BONK_OPERATOR_LESS_OR_EQUAL_THAN,
    BONK_OPERATOR_GREATER_OR_EQUAL_THAN,
    BONK_OPERATOR_CYCLE,
    BONK_OPERATOR_CHECK,
    BONK_OPERATOR_PRINT,
    BONK_OPERATOR_BONK,
    BONK_OPERATOR_BREK,
    BONK_OPERATOR_BAMS,
    BONK_OPERATOR_AND,
    BONK_OPERATOR_OR,
    BONK_OPERATOR_REBONK,
    BONK_OPERATOR_INVALID
};

extern const char* OPERATOR_TYPE_NAMES[];

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
#include "bonk/parsing/parser_position.hpp"

namespace bonk {

struct TreeNode {
    TreeNodeType type = TREE_NODE_TYPE_INVALID;
    ParserPosition* source_position = nullptr;

    TreeNode() = default;
    virtual ~TreeNode() = default;

    virtual void accept(ASTVisitor* visitor) {}
};

struct TreeNodeBlockDefinition : TreeNode {

    std::unique_ptr<TreeNodeIdentifier> block_name;
    std::unique_ptr<TreeNodeList> body;
    bool is_promise = false;

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
    OperatorType oper_type = BONK_OPERATOR_INVALID;
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