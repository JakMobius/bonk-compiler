
#include "types.hpp"
#include "type_visitor.hpp"

bool bonk::Type::is(TypeKind other_kind) const {
    return kind == other_kind;
}

bool bonk::Type::is(TrivialTypeKind type) const {
    if (kind != TypeKind::primitive)
        return false;
    return ((const TrivialType*)this)->trivial_kind == type;
}

bonk::HiveType::HiveType() {
    kind = TypeKind::hive;
}

bonk::BlokType::BlokType() {
    kind = TypeKind::blok;
}

bonk::TrivialType::TrivialType() {
    kind = TypeKind::primitive;
}

bonk::ManyType::ManyType() {
    kind = TypeKind::many;
}

bonk::ErrorType::ErrorType() {
    kind = TypeKind::error;
}

bonk::ExternalType::ExternalType() {
    kind = TypeKind::external;
}

bonk::NullType::NullType() {
    kind = TypeKind::null;
}

bool bonk::Type::operator==(const bonk::Type& other) const {
    return kind == other.kind;
}

bool bonk::Type::operator!=(const bonk::Type& other) const {
    return !(*this == other);
}

bool bonk::HiveType::operator==(const bonk::Type& other) const {
    if (other.kind != TypeKind::hive)
        return false;
    return hive_definition == ((const HiveType*)&other)->hive_definition;
}
bool bonk::BlokType::operator==(const bonk::Type& other) const {
    if (other.kind != TypeKind::blok)
        return false;
    return parameters == ((const BlokType*)&other)->parameters &&
           *return_type == *((const BlokType*)&other)->return_type;
}

bool bonk::TrivialType::operator==(const bonk::Type& other) const {
    if (other.kind != TypeKind::primitive)
        return false;
    return trivial_kind == ((const TrivialType*)&other)->trivial_kind;
}

bool bonk::ManyType::operator==(const bonk::Type& other) const {
    if (other.kind != TypeKind::many)
        return false;
    return *element_type == *((const ManyType*)&other)->element_type;
}

bool bonk::ErrorType::operator==(const bonk::Type& other) const {
    return other.kind == TypeKind::error;
}

bool bonk::ExternalType::operator==(const bonk::Type& other) const {
    if (other.kind == TypeKind::external) {
        return *get_resolved() == *((bonk::ExternalType&)other).get_resolved();
    }
    return *get_resolved() == other;
}

bool bonk::NullType::operator==(const bonk::Type& other) const {
    return other.kind == TypeKind::null;
}

bonk::Type* bonk::ExternalType::get_resolved() const {
    if (!resolved) {
        resolved = resolver->resolve();
        resolver = nullptr;

        if (!resolved) {
            resolved = std::make_unique<ErrorType>();
        }

        return resolved.get();
    }
    return resolved.get();
}

bool bonk::Type::allows_unary_operation(bonk::OperatorType operator_type) const {
    return false;
}

bool bonk::ExternalType::allows_unary_operation(bonk::OperatorType operator_type) const {
    return get_resolved()->allows_unary_operation(operator_type);
}

bool bonk::Type::allows_binary_operation(bonk::OperatorType operator_type, Type* other_type) const {
    return false;
}

std::ostream& bonk::operator<<(std::ostream& stream, const bonk::Type& type) {
    StdOutputStream std_stream{stream};
    TypePrinter printer(std_stream);
    type.accept(&printer);
    return stream;
}

bool bonk::HiveType::allows_binary_operation(bonk::OperatorType operator_type,
                                             bonk::Type* other_type) const {

    if (other_type->kind == TypeKind::null || *other_type == *this) {
        return operator_type == OperatorType::o_equal ||
               operator_type == OperatorType::o_not_equal ||
               operator_type == OperatorType::o_assign;
    }

    return false;
}

bool bonk::BlokType::allows_binary_operation(bonk::OperatorType operator_type,
                                             bonk::Type* other_type) const {
    // Allow assignment
    if (operator_type == OperatorType::o_assign && *other_type == *this) {
        return true;
    }
    return false;
}

bool bonk::TrivialType::allows_binary_operation(bonk::OperatorType operator_type,
                                                Type* other_type) const {
    switch (trivial_kind) {
    case TrivialTypeKind::t_never:
        return false;
    case TrivialTypeKind::t_unset:
    case TrivialTypeKind::t_nothing:
        return false;
    case TrivialTypeKind::t_buul:
    case TrivialTypeKind::t_shrt:
    case TrivialTypeKind::t_nubr:
    case TrivialTypeKind::t_long:
    case TrivialTypeKind::t_flot:
    case TrivialTypeKind::t_dabl:
        if (*other_type == *this) {
            return (operator_type == OperatorType::o_plus ||
                    operator_type == OperatorType::o_minus ||
                    operator_type == OperatorType::o_multiply ||
                    operator_type == OperatorType::o_divide ||
                    operator_type == OperatorType::o_assign ||
                    operator_type == OperatorType::o_plus_assign ||
                    operator_type == OperatorType::o_minus_assign ||
                    operator_type == OperatorType::o_multiply_assign ||
                    operator_type == OperatorType::o_divide_assign ||
                    operator_type == OperatorType::o_equal ||
                    operator_type == OperatorType::o_not_equal ||
                    operator_type == OperatorType::o_less ||
                    operator_type == OperatorType::o_less_equal ||
                    operator_type == OperatorType::o_greater ||
                    operator_type == OperatorType::o_greater_equal ||
                    operator_type == OperatorType::o_and || operator_type == OperatorType::o_or);
        } else if (other_type->is(TrivialTypeKind::t_nothing)) {
            // As 'and' and 'or' operations are used to express the if statement,
            // we allow them to be used with nothing type, because code block
            // has nothing type
            return operator_type == OperatorType::o_and || operator_type == OperatorType::o_or;
        }
    case TrivialTypeKind::t_strg:
        if (other_type->kind == TypeKind::null) {
            return operator_type == OperatorType::o_equal ||
                   operator_type == OperatorType::o_not_equal ||
                   operator_type == OperatorType::o_assign;
        }

        return *other_type == *this &&
               (operator_type == OperatorType::o_plus || operator_type == OperatorType::o_assign ||
                operator_type == OperatorType::o_plus_assign ||
                operator_type == OperatorType::o_equal ||
                operator_type == OperatorType::o_not_equal);
    }
    return false;
}

bool bonk::TrivialType::allows_unary_operation(bonk::OperatorType operator_type) const {
    switch (trivial_kind) {
    case TrivialTypeKind::t_shrt:
    case TrivialTypeKind::t_nubr:
    case TrivialTypeKind::t_long:
    case TrivialTypeKind::t_flot:
    case TrivialTypeKind::t_dabl:
        return operator_type == OperatorType::o_plus || operator_type == OperatorType::o_minus;
    case TrivialTypeKind::t_buul:
    case TrivialTypeKind::t_strg:
    case TrivialTypeKind::t_unset:
    case TrivialTypeKind::t_never:
    case TrivialTypeKind::t_nothing:
        return false;
    }
    return false;
}

bool bonk::ManyType::allows_binary_operation(bonk::OperatorType operator_type,
                                             Type* other_type) const {
    if (other_type->kind == TypeKind::null) {
        return operator_type == OperatorType::o_equal ||
               operator_type == OperatorType::o_not_equal ||
               operator_type == OperatorType::o_assign;
    }

    return (operator_type == OperatorType::o_plus_assign && *other_type == *element_type) ||
           (operator_type == OperatorType::o_assign && *other_type == *this);
}

bool bonk::ExternalType::allows_binary_operation(bonk::OperatorType operator_type,
                                                 bonk::Type* other_type) const {
    return get_resolved()->allows_binary_operation(operator_type, other_type);
}

bool bonk::NullType::allows_binary_operation(bonk::OperatorType operator_type,
                                             bonk::Type* other_type) const {
    if (!other_type->is(TypeKind::null) && !other_type->is(TypeKind::hive) &&
        !other_type->is(TypeKind::many) && !other_type->is(TrivialTypeKind::t_strg)) {
        return false;
    }

    return operator_type == OperatorType::o_equal || operator_type == OperatorType::o_not_equal;
}

void bonk::HiveType::accept(ConstTypeVisitor* visitor) const {
    visitor->visit(this);
}

void bonk::BlokType::accept(ConstTypeVisitor* visitor) const {
    visitor->visit(this);
}

void bonk::TrivialType::accept(ConstTypeVisitor* visitor) const {
    visitor->visit(this);
}

void bonk::ManyType::accept(ConstTypeVisitor* visitor) const {
    visitor->visit(this);
}

void bonk::ErrorType::accept(ConstTypeVisitor* visitor) const {
    visitor->visit(this);
}

void bonk::ExternalType::accept(bonk::ConstTypeVisitor* visitor) const {
    visitor->visit(this);
}

void bonk::NullType::accept(bonk::ConstTypeVisitor* visitor) const {
    visitor->visit(this);
}
