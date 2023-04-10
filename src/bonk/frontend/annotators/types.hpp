#pragma once

#include "bonk/frontend/parsing/lexic/lexer.hpp"
#include "bonk/frontend/ast/ast.hpp"

namespace bonk {

class ConstTypeVisitor;

enum class TypeKind { unset, primitive, hive, blok, many, error, external, null };

class Type {
  public:
    TypeKind kind = TypeKind::unset;

    virtual bool operator==(const Type& other) const;
    virtual bool allows_binary_operation(OperatorType operator_type, Type* other_type) const;
    virtual bool allows_unary_operation(OperatorType operator_type) const;
    virtual void accept(ConstTypeVisitor* visitor) const = 0;
    virtual ~Type() = default;
    bool operator!=(const Type& other) const;

    friend std::ostream& operator<<(std::ostream& stream, const Type& type);

    bool is(TypeKind kind) const;
    bool is(TrivialTypeKind type) const;
};

class HiveType : public Type {
  public:
    TreeNodeHiveDefinition* hive_definition = nullptr;
    HiveType();
    bool operator==(const Type& other) const override;
    bool allows_binary_operation(OperatorType operator_type, Type* other_type) const override;
    void accept(ConstTypeVisitor* visitor) const override;
};

class BlokType : public Type {
  public:
    std::list<TreeNodeVariableDefinition*> parameters;
    std::unique_ptr<Type> return_type;
    BlokType();
    bool operator==(const Type& other) const override;
    bool allows_binary_operation(OperatorType operator_type, Type* other_type) const override;
    void accept(ConstTypeVisitor* visitor) const override;
};

class TrivialType : public Type {
  public:
    TrivialTypeKind trivial_kind = TrivialTypeKind::t_unset;
    TrivialType();
    bool operator==(const Type& other) const override;
    bool allows_binary_operation(OperatorType operator_type, Type* other_type) const override;
    bool allows_unary_operation(OperatorType operator_type) const override;
    void accept(ConstTypeVisitor* visitor) const override;
};

class ManyType : public Type {
  public:
    std::unique_ptr<Type> element_type;
    ManyType();
    bool operator==(const Type& other) const override;
    bool allows_binary_operation(OperatorType operator_type, Type* other_type) const override;
    void accept(ConstTypeVisitor* visitor) const override;
};

class ErrorType : public Type {
  public:
    ErrorType();
    bool operator==(const Type& other) const override;
    void accept(ConstTypeVisitor* visitor) const override;
};

class ExternalTypeResolver {
  public:
    virtual ~ExternalTypeResolver() = default;
    virtual std::unique_ptr<Type> resolve() = 0;
    virtual std::unique_ptr<ExternalTypeResolver> clone() = 0;
};

class ExternalType : public Type {
  public:
    mutable std::unique_ptr<ExternalTypeResolver> resolver;
    mutable std::unique_ptr<Type> resolved;

    bonk::Type* get_resolved() const;

    ExternalType();
    bool operator==(const Type& other) const override;
    bool allows_binary_operation(OperatorType operator_type, Type* other_type) const override;
    bool allows_unary_operation(OperatorType operator_type) const override;
    void accept(ConstTypeVisitor* visitor) const override;
};

class NullType : public Type {
  public:
    NullType();
    bool operator==(const Type& other) const override;
    bool allows_binary_operation(OperatorType operator_type, Type* other_type) const override;
    void accept(ConstTypeVisitor* visitor) const override;
};
}