
#include "types.hpp"
#include "type_visitor.hpp"

bonk::HiveType::HiveType() {
    kind = TypeKind::hive;
}

bonk::BlokType::BlokType() {
    kind = TypeKind::blok;
}

bonk::TrivialType::TrivialType() {
    kind = TypeKind::primitive;
}

bonk::NothingType::NothingType() {
    kind = TypeKind::nothing;
}

bonk::ManyType::ManyType() {
    kind = TypeKind::many;
}

bonk::NeverType::NeverType() {
    kind = TypeKind::never;
}

bonk::ErrorType::ErrorType() {
    kind = TypeKind::error;
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
    return primitive_type == ((const TrivialType*)&other)->primitive_type;
}

bool bonk::NothingType::operator==(const bonk::Type& other) const {
    return other.kind == TypeKind::nothing;
}

bool bonk::ManyType::operator==(const bonk::Type& other) const {
    if (other.kind != TypeKind::many)
        return false;
    return *element_type == *((const ManyType*)&other)->element_type;
}

bool bonk::NeverType::operator==(const bonk::Type& other) const {
    return other.kind == TypeKind::never;
}

bool bonk::ErrorType::operator==(const bonk::Type& other) const {
    return other.kind == TypeKind::error;
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

bool bonk::Type::allows_unary_operation(bonk::OperatorType operator_type) const {
    return false;
}

bool bonk::HiveType::allows_binary_operation(bonk::OperatorType operator_type,
                                             bonk::Type* other_type) const {
    // Allow assignment
    if (operator_type == OperatorType::o_assign && *other_type == *this) {
        return true;
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
    switch (primitive_type) {

    case PrimitiveType::t_unset:
        return false;
    case PrimitiveType::t_buul:
    case PrimitiveType::t_shrt:
    case PrimitiveType::t_nubr:
    case PrimitiveType::t_long:
    case PrimitiveType::t_flot:
    case PrimitiveType::t_dabl:
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
        } else if (other_type->kind == TypeKind::nothing) {
            // As 'and' and 'or' operations are used to express the if statement,
            // we allow them to be used with nothing type, because code block
            // has nothing type
            return operator_type == OperatorType::o_and || operator_type == OperatorType::o_or;
        }
    case PrimitiveType::t_strg:
        return *other_type == *this &&
               (operator_type == OperatorType::o_plus || operator_type == OperatorType::o_assign ||
                operator_type == OperatorType::o_plus_assign ||
                operator_type == OperatorType::o_equal ||
                operator_type == OperatorType::o_not_equal);
    }
}

bool bonk::TrivialType::allows_unary_operation(bonk::OperatorType operator_type) const {
    switch (primitive_type) {
    case PrimitiveType::t_shrt:
    case PrimitiveType::t_nubr:
    case PrimitiveType::t_long:
    case PrimitiveType::t_flot:
    case PrimitiveType::t_dabl:
        return operator_type == OperatorType::o_plus || operator_type == OperatorType::o_minus;
    case PrimitiveType::t_buul:
    case PrimitiveType::t_strg:
    case PrimitiveType::t_unset:
        return false;
    }
}

bool bonk::ManyType::allows_binary_operation(bonk::OperatorType operator_type,
                                             Type* other_type) const {
    return (operator_type == OperatorType::o_plus_assign && *other_type == *element_type) ||
           (operator_type == OperatorType::o_assign && *other_type == *this);
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

void bonk::NothingType::accept(ConstTypeVisitor* visitor) const {
    visitor->visit(this);
}

void bonk::ManyType::accept(ConstTypeVisitor* visitor) const {
    visitor->visit(this);
}

void bonk::NeverType::accept(ConstTypeVisitor* visitor) const {
    visitor->visit(this);
}

void bonk::ErrorType::accept(ConstTypeVisitor* visitor) const {
    visitor->visit(this);
}