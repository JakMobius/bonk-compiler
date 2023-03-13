
#include "types.hpp"

bool bonk::Type::operator==(const bonk::Type& other) const {
    return kind == other.kind;
}

bool bonk::Type::operator!=(const bonk::Type& other) const {
    return !(*this == other);
}

bool bonk::Type::allows_binary_operation(bonk::OperatorType operator_type, Type* other_type) const {
    return false;
}

void bonk::Type::print(std::ostream& stream) const {
    stream << "unknown";
}

std::ostream& bonk::operator<<(std::ostream& stream, const bonk::Type& type) {
    type.print(stream);
    return stream;
}

std::unique_ptr<bonk::Type> bonk::Type::shallow_copy() const {
    return {};
}

bool bonk::Type::allows_unary_operation(bonk::OperatorType operator_type) const {
    return false;
}
int bonk::Type::footprint() {
    return 0;
}

bonk::HiveType::HiveType() {
    kind = TypeKind::hive;
}

bool bonk::HiveType::operator==(const bonk::Type& other) const {
    if (other.kind != TypeKind::hive)
        return false;
    return hive_definition == ((const HiveType*)&other)->hive_definition;
}

void bonk::HiveType::print(std::ostream& stream) const {
    stream << hive_definition->hive_name->identifier_text;
}

bool bonk::HiveType::allows_binary_operation(bonk::OperatorType operator_type,
                                             bonk::Type* other_type) const {
    // Allow assignment
    if (operator_type == OperatorType::o_assign && *other_type == *this) {
        return true;
    }
    return false;
}

std::unique_ptr<bonk::Type> bonk::HiveType::shallow_copy() const {
    auto copy = std::make_unique<HiveType>();
    copy->hive_definition = hive_definition;
    return copy;
}
int bonk::HiveType::footprint() {
    return 8;
}

bonk::BlokType::BlokType() {
    kind = TypeKind::blok;
}

bool bonk::BlokType::operator==(const bonk::Type& other) const {
    if (other.kind != TypeKind::blok)
        return false;
    return parameters == ((const BlokType*)&other)->parameters &&
           *return_type == *((const BlokType*)&other)->return_type;
}

void bonk::BlokType::print(std::ostream& stream) const {
    stream << "blok[";
    bool first = true;
    for (auto& parameter : parameters) {
        if (!first) {
            stream << ", ";
        }
        first = false;
        stream << parameter->variable_name->identifier_text << ": ";

        // TODO: figure out how to print the type
        stream << "<type>";
    }
    stream << "] -> ";
    return_type->print(stream);
}

bool bonk::BlokType::allows_binary_operation(bonk::OperatorType operator_type,
                                             bonk::Type* other_type) const {
    // Allow assignment
    if (operator_type == OperatorType::o_assign && *other_type == *this) {
        return true;
    }
    return false;
}

std::unique_ptr<bonk::Type> bonk::BlokType::shallow_copy() const {
    auto copy = std::make_unique<BlokType>();
    copy->parameters = parameters;
    copy->return_type = return_type->shallow_copy();
    return copy;
}
int bonk::BlokType::footprint() {
    return 8;
}

bonk::TrivialType::TrivialType() {
    kind = TypeKind::primitive;
}

bool bonk::TrivialType::operator==(const bonk::Type& other) const {
    if (other.kind != TypeKind::primitive)
        return false;
    return primitive_type == ((const TrivialType*)&other)->primitive_type;
}

std::unique_ptr<bonk::Type> bonk::TrivialType::shallow_copy() const {
    auto copy = std::make_unique<TrivialType>();
    copy->primitive_type = primitive_type;
    return copy;
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
        } else if(other_type->kind == TypeKind::nothing) {
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

void bonk::TrivialType::print(std::ostream& stream) const {
    stream << BONK_PRIMITIVE_TYPE_NAMES[(int)primitive_type];
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
int bonk::TrivialType::footprint() {
    switch (primitive_type) {
        case PrimitiveType::t_unset:
                return 0;
        case PrimitiveType::t_buul:
                return 1;
        case PrimitiveType::t_shrt:
                return 2;
        case PrimitiveType::t_nubr:
        case PrimitiveType::t_flot:
                return 4;
        case PrimitiveType::t_long:
        case PrimitiveType::t_dabl:
        case PrimitiveType::t_strg:
                return 8;
    }
    return 0;
}

bonk::NothingType::NothingType() {
    kind = TypeKind::nothing;
}

bool bonk::NothingType::operator==(const bonk::Type& other) const {
    return other.kind == TypeKind::nothing;
}

void bonk::NothingType::print(std::ostream& stream) const {
    stream << "nothing";
}

std::unique_ptr<bonk::Type> bonk::NothingType::shallow_copy() const {
    return std::make_unique<NothingType>();
}
int bonk::NothingType::footprint() {
    return 0;
}

bonk::ManyType::ManyType() {
    kind = TypeKind::many;
}

bool bonk::ManyType::operator==(const bonk::Type& other) const {
    if (other.kind != TypeKind::many)
        return false;
    return *element_type == *((const ManyType*)&other)->element_type;
}
bool bonk::ManyType::allows_binary_operation(bonk::OperatorType operator_type,
                                             Type* other_type) const {
    return (operator_type == OperatorType::o_plus_assign && *other_type == *element_type) ||
           (operator_type == OperatorType::o_assign && *other_type == *this);
}
void bonk::ManyType::print(std::ostream& stream) const {
    stream << "many ";
    element_type->print(stream);
}

std::unique_ptr<bonk::Type> bonk::ManyType::shallow_copy() const {
    auto copy = std::make_unique<ManyType>();
    copy->element_type = element_type->shallow_copy();
    return copy;
}
int bonk::ManyType::footprint() {
    return 8;
}

bonk::NeverType::NeverType() {
    kind = TypeKind::never;
}

bool bonk::NeverType::operator==(const bonk::Type& other) const {
    return other.kind == TypeKind::never;
}

void bonk::NeverType::print(std::ostream& stream) const {
    stream << "never";
}

std::unique_ptr<bonk::Type> bonk::NeverType::shallow_copy() const {
    return std::make_unique<NeverType>();
}
int bonk::NeverType::footprint() {
    return 0;
}

bonk::ErrorType::ErrorType() {
    kind = TypeKind::error;
}

bool bonk::ErrorType::operator==(const bonk::Type& other) const {
    return other.kind == TypeKind::error;
}

void bonk::ErrorType::print(std::ostream& stream) const {
    stream << "error type";
}

std::unique_ptr<bonk::Type> bonk::ErrorType::shallow_copy() const {
    return std::make_unique<ErrorType>();
}
int bonk::ErrorType::footprint() {
    return 0;
}
