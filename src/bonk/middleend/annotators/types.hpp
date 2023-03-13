#pragma once

#include "bonk/parsing/lexic/lexical_analyzer.hpp"

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
    virtual int footprint();
    bool operator!=(const Type& other) const;

    friend std::ostream& operator<<(std::ostream& stream, const Type& type);
};

class HiveType : public Type {
  public:
    TreeNodeHiveDefinition* hive_definition = nullptr;
    HiveType();
    bool operator==(const Type& other) const override;
    void print(std::ostream& stream) const override;
    bool allows_binary_operation(OperatorType operator_type, Type* other_type) const override;
    std::unique_ptr<Type> shallow_copy() const override;
    int footprint() override;
};

class BlokType : public Type {
  public:
    std::list<TreeNodeVariableDefinition*> parameters;
    std::unique_ptr<Type> return_type;
    BlokType();
    bool operator==(const Type& other) const override;
    void print(std::ostream& stream) const override;
    bool allows_binary_operation(OperatorType operator_type, Type* other_type) const override;
    std::unique_ptr<Type> shallow_copy() const override;
    int footprint() override;
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
    int footprint() override;
};

class ManyType : public Type {
  public:
    std::unique_ptr<Type> element_type;
    ManyType();
    bool operator==(const Type& other) const override;
    bool allows_binary_operation(OperatorType operator_type, Type* other_type) const override;
    void print(std::ostream& stream) const override;
    std::unique_ptr<Type> shallow_copy() const override;
    int footprint() override;
};

class NothingType : public Type {
  public:
    NothingType();
    bool operator==(const Type& other) const override;
    void print(std::ostream& stream) const override;
    std::unique_ptr<Type> shallow_copy() const override;
    int footprint() override;
};

class NeverType : public Type {
  public:
    NeverType();
    bool operator==(const Type& other) const override;
    void print(std::ostream& stream) const override;
    std::unique_ptr<Type> shallow_copy() const override;
    int footprint() override;
};

class ErrorType : public Type {
  public:
    ErrorType();
    bool operator==(const Type& other) const override;
    void print(std::ostream& stream) const override;
    std::unique_ptr<Type> shallow_copy() const override;
    int footprint() override;
};

}