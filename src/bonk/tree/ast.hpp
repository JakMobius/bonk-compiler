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

    TreeNodeIdentifier* block_name = nullptr;
    TreeNodeList* body = nullptr;
    bool is_promise = false;

    TreeNodeBlockDefinition();

    ~TreeNodeBlockDefinition() override;

    void accept(ASTVisitor* visitor) override;
};

struct TreeNodeCall : TreeNode {

    TreeNodeIdentifier* call_function = nullptr;
    TreeNodeList* call_parameters = nullptr;

    TreeNodeCall(TreeNodeIdentifier* function, TreeNodeList* parameters);

    ~TreeNodeCall() override;

    void accept(ASTVisitor* visitor) override;
};

struct TreeNodeCallParameter : TreeNode {

    TreeNodeIdentifier* parameter_name = nullptr;
    TreeNode* parameter_value = nullptr;

    TreeNodeCallParameter(TreeNodeIdentifier* name, TreeNode* value);

    ~TreeNodeCallParameter() override;

    void accept(ASTVisitor* visitor) override;
};

struct TreeNodeCheck : TreeNode {

    TreeNode* condition = nullptr;
    TreeNodeList* check_body = nullptr;
    TreeNodeList* or_body = nullptr;

    TreeNodeCheck();

    ~TreeNodeCheck() override;

    void accept(ASTVisitor* visitor) override;
};

struct TreeNodeCycle : TreeNode {
    TreeNodeList* body = nullptr;

    TreeNodeCycle();

    void accept(ASTVisitor* visitor) override;
};

struct TreeNodeIdentifier : TreeNode {
    std::string variable_name;

    explicit TreeNodeIdentifier(const std::string& name);

    bool contents_equal(TreeNodeIdentifier* other);

    void print(FILE* file) const;

    void accept(ASTVisitor* visitor) override;
};

struct TreeNodeList : TreeNode {

    std::list<TreeNode*> list;

    TreeNodeList();
    ~TreeNodeList() override;

    void accept(ASTVisitor* visitor) override;
};

struct TreeNodeNumber : TreeNode {
    long long integer_value;
    long double float_value;

    TreeNodeNumber(long double float_value, long long integer_value);

    void accept(ASTVisitor* visitor) override;
};

struct TreeNodeOperator : TreeNode {
    OperatorType oper_type = BONK_OPERATOR_INVALID;
    TreeNode* left = nullptr;
    TreeNode* right = nullptr;

    explicit TreeNodeOperator(OperatorType oper);

    void accept(ASTVisitor* visitor) override;
};

struct TreeNodeVariableDefinition : TreeNode {
    bool is_contextual = false;
    TreeNodeIdentifier* variable_name = nullptr;
    TreeNode* variable_value = nullptr;

    TreeNodeVariableDefinition(bool contextual, TreeNodeIdentifier* identifier);
    ~TreeNodeVariableDefinition() override;

    void accept(ASTVisitor* visitor) override;
};

}