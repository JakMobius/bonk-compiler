#pragma once

#include "types.hpp"
#include "utils/streams.hpp"
namespace bonk {

class ConstTypeVisitor {
  public:
    virtual void visit(const HiveType* type);
    virtual void visit(const BlokType* type);
    virtual void visit(const TrivialType* type);
    virtual void visit(const ManyType* type);
    virtual void visit(const NothingType* type);
    virtual void visit(const NeverType* type);
    virtual void visit(const ErrorType* type);
};

class TypeCloner : public ConstTypeVisitor {
  public:
    std::unique_ptr<Type> result;
    void visit(const HiveType* type) override;
    void visit(const BlokType* type) override;
    void visit(const TrivialType* type) override;
    void visit(const ManyType* type) override;
    void visit(const NothingType* type) override;
    void visit(const NeverType* type) override;
    void visit(const ErrorType* type) override;

    template <typename T>
    std::unique_ptr<T> clone(T* type) {
        if(!type) return nullptr;
        type->accept(this);
        return std::unique_ptr<T>(static_cast<T*>(result.release()));
    }
};

class TypePrinter : public ConstTypeVisitor {
  public:
    const OutputStream& stream;

    TypePrinter(const OutputStream& stream) : stream(stream) {}

    void visit(const HiveType* type) override;
    void visit(const BlokType* type) override;
    void visit(const TrivialType* type) override;
    void visit(const ManyType* type) override;
    void visit(const NothingType* type) override;
    void visit(const NeverType* type) override;
    void visit(const ErrorType* type) override;
};

class FootprintCounter : public ConstTypeVisitor {
    int pointer_size = 8;
  public:
    int footprint = 0;

    void visit(const HiveType* type) override;
    void visit(const BlokType* type) override;
    void visit(const TrivialType* type) override;
    void visit(const ManyType* type) override;
    void visit(const NothingType* type) override;
    void visit(const NeverType* type) override;
    void visit(const ErrorType* type) override;

    int get_footprint(Type* type);
};

class NeverSearchVisitor : public ConstTypeVisitor {
    public:
    bool result = false;

    void visit(const HiveType* type) override;
    void visit(const BlokType* type) override;
    void visit(const TrivialType* type) override;
    void visit(const ManyType* type) override;
    void visit(const NothingType* type) override;
    void visit(const NeverType* type) override;
    void visit(const ErrorType* type) override;

    bool search(Type* type);
};

} // namespace bonk