#pragma once

namespace bonk {

class TypeAnnotator;
class MiddleEnd;

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

/* This class performs type-checking and annotates the AST tree with types.
 * It requires `symbol_table` to be filled in by basic_symbol_annotator.
 * In turn, it populates the `symbol_table` with hive field symbols and
 * function arguments, as it's impossible to recognize them without
 * type information. */

class TypeAnnotator : ASTVisitor {

  public:
    MiddleEnd& middle_end;

    explicit TypeAnnotator(MiddleEnd& middle_end);
    ~TypeAnnotator() = default;

    Type* infer_type(TreeNode* node);

    void visit(TreeNodeHiveDefinition* node) override;
    void visit(TreeNodeVariableDefinition* node) override;
    void visit(TreeNodeBlockDefinition* node) override;
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
    void annotate_ast(TreeNode* ast);
};

} // namespace bonk