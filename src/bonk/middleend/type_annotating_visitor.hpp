#pragma once

namespace bonk {

class TypeAnnotatingVisitor;

}

#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include "bonk/tree/ast_visitor.hpp"

namespace bonk {

enum class TypeKind { unset, primitive, hive, blok, many, nothing, never, error };

class Type {
  public:
    TypeKind kind = TypeKind::unset;

    virtual bool operator==(const Type& other) const;
    virtual bool allows_binary_operation(OperatorType operator_type, Type* other_type) const;
    virtual bool allows_unary_operation(OperatorType operator_type) const;
    virtual void print(std::ostream& stream) const;
    virtual std::unique_ptr<Type> shallow_copy() const;
    bool operator!=(const Type& other) const;

    friend std::ostream& operator<<(std::ostream& stream, const Type& type);
};

class HiveType : public Type {
  public:
    TreeNodeHiveDefinition* hive_definition = nullptr;
    HiveType();
    bool operator==(const Type& other) const override;
    void print(std::ostream& stream) const override;
    std::unique_ptr<Type> shallow_copy() const override;
};

class BlokType : public Type {
  public:
    std::list<TreeNodeVariableDefinition*> parameters;
    std::unique_ptr<Type> return_type;
    BlokType();
    bool operator==(const Type& other) const override;
    void print(std::ostream& stream) const override;
    std::unique_ptr<Type> shallow_copy() const override;
};

class TrivialType : public Type {
  public:
    PrimitiveType primitive_type = PrimitiveType::t_unset;
    TrivialType();
    bool operator==(const Type& other) const override;
    bool allows_binary_operation(OperatorType operator_type, Type* other_type) const override;
    bool allows_unary_operation(OperatorType operator_type) const override;
    void print(std::ostream& stream) const override;
    std::unique_ptr<Type> shallow_copy() const override;
};

class ManyType : public Type {
  public:
    std::unique_ptr<Type> element_type;
    ManyType();
    bool operator==(const Type& other) const override;
    bool allows_binary_operation(OperatorType operator_type, Type* other_type) const override;
    void print(std::ostream& stream) const override;
    std::unique_ptr<Type> shallow_copy() const override;
};

class NothingType : public Type {
  public:
    NothingType();
    bool operator==(const Type& other) const override;
    void print(std::ostream& stream) const override;
    std::unique_ptr<Type> shallow_copy() const override;
};

class NeverType : public Type {
  public:
    NeverType();
    bool operator==(const Type& other) const override;
    void print(std::ostream& stream) const override;
    std::unique_ptr<Type> shallow_copy() const override;
};

class ErrorType : public Type {
  public:
    ErrorType();
    bool operator==(const Type& other) const override;
    void print(std::ostream& stream) const override;
    std::unique_ptr<Type> shallow_copy() const override;
};

class Scope {
  public:
    std::unordered_map<std::string_view, int> variables;
};

class NameResolver {
  public:
    virtual TreeNode* get_name_definition(std::string_view name) = 0;
};

class ScopedNameResolver : public NameResolver {
    std::unordered_map<TreeNode*, Type> inferred_types;
    std::unordered_map<int, TreeNode*> name_definitions;
    std::unordered_map<TreeNode*, TreeNode*> identifier_definitions;
    std::vector<Scope> scopes;
    int total_names = 0;

  public:
    ScopedNameResolver();

    void padding();
    void push_scope();
    void pop_scope();
    TreeNode* get_name_definition(std::string_view name) override;
    void define_variable(TreeNode* definition);
    TreeNode* get_definition(int name_id);
};

class HiveFieldNameResolver : public NameResolver {
    TreeNodeHiveDefinition* hive_definition = nullptr;

  public:
    explicit HiveFieldNameResolver(TreeNodeHiveDefinition* hive_definition);
    TreeNode* get_name_definition(std::string_view name) override;
};

class FunctionParameterNameResolver : public NameResolver {
    BlokType* called_function = nullptr;

  public:
    explicit FunctionParameterNameResolver(BlokType* called_function);
    TreeNode* get_name_definition(std::string_view name) override;
};

class TypeAnnotatingVisitor : public ASTVisitor {
    Type* callee_type = nullptr;
    NameResolver* active_name_resolver;

  public:
    Compiler& linked_compiler;
    ScopedNameResolver scoped_name_resolver;
    std::unordered_map<TreeNode*, Type*> type_cache {};
    std::unordered_set<std::unique_ptr<Type>> type_storage {};


    explicit TypeAnnotatingVisitor(Compiler& linked_compiler);
    ~TypeAnnotatingVisitor() = default;

    Type* infer_type(TreeNode* node);

    void visit(TreeNodeCodeBlock* node) override;
    void visit(TreeNodeHiveDefinition* node) override;
    void visit(TreeNodeVariableDefinition* node) override;
    void visit(TreeNodeBlockDefinition* node) override;
    void visit(TreeNodeLoopStatement* node) override;
    void visit(TreeNodeHiveAccess* node) override;
    void visit(TreeNodeCall* node) override;
    void visit(TreeNodeParameterListItem* node) override;

    void visit(TreeNodeArrayConstant* node) override;
    void visit(TreeNodeNumberConstant* node) override;
    void visit(TreeNodeStringConstant* node) override;
    void visit(TreeNodeBinaryOperation* node) override;
    void visit(TreeNodeUnaryOperation* node) override;
    void visit(TreeNodePrimitiveType* node) override;
    void visit(TreeNodeManyType* node) override;
    void visit(TreeNodeBonkStatement* node) override;
};

} // namespace bonk