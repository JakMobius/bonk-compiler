
#include "type_visitor.hpp"

void bonk::ConstTypeVisitor::visit(const bonk::HiveType* type) {
}

void bonk::ConstTypeVisitor::visit(const bonk::BlokType* type) {
    type->return_type->accept(this);
}

void bonk::ConstTypeVisitor::visit(const bonk::TrivialType* type) {
}

void bonk::ConstTypeVisitor::visit(const bonk::ManyType* type) {
    type->element_type->accept(this);
}

void bonk::ConstTypeVisitor::visit(const bonk::NothingType* type) {
}

void bonk::ConstTypeVisitor::visit(const bonk::NeverType* type) {
}

void bonk::ConstTypeVisitor::visit(const bonk::ErrorType* type) {
}

void bonk::TypeCloner::visit(const bonk::HiveType* type) {
    auto copy = std::make_unique<HiveType>();
    copy->hive_definition = type->hive_definition;
    result = std::move(copy);
}

void bonk::TypeCloner::visit(const bonk::BlokType* type) {
    auto copy = std::make_unique<BlokType>();
    copy->return_type = clone(type->return_type.get());
    copy->parameters = type->parameters;
    result = std::move(copy);
}

void bonk::TypeCloner::visit(const bonk::TrivialType* type) {
    auto copy = std::make_unique<TrivialType>();
    copy->primitive_type = type->primitive_type;
    result = std::move(copy);
}

void bonk::TypeCloner::visit(const bonk::ManyType* type) {
    auto copy = std::make_unique<ManyType>();
    copy->element_type = clone(type->element_type.get());
    result = std::move(copy);
}

void bonk::TypeCloner::visit(const bonk::NothingType* type) {
    result = std::make_unique<NothingType>();
}

void bonk::TypeCloner::visit(const bonk::NeverType* type) {
    result = std::make_unique<NeverType>();
}

void bonk::TypeCloner::visit(const bonk::ErrorType* type) {
    result = std::make_unique<ErrorType>();
}

void bonk::TypePrinter::visit(const bonk::HiveType* type) {
    stream.get_stream() << type->hive_definition->hive_name->identifier_text;
}

void bonk::TypePrinter::visit(const bonk::BlokType* type) {
    stream.get_stream() << "blok[";
    bool first = true;
    for (auto& parameter : type->parameters) {
        if (!first) {
            stream.get_stream() << ", ";
        }
        first = false;
        stream.get_stream() << parameter->variable_name->identifier_text << ": ";

        // TODO: figure out how to print the type
        stream.get_stream() << "<type>";
    }
    stream.get_stream() << "] -> ";

    type->return_type->accept(this);
}

void bonk::TypePrinter::visit(const bonk::TrivialType* type) {
    stream.get_stream() << BONK_PRIMITIVE_TYPE_NAMES[(int)type->primitive_type];
}

void bonk::TypePrinter::visit(const bonk::ManyType* type) {
    stream.get_stream() << "many ";
    type->element_type->accept(this);
}

void bonk::TypePrinter::visit(const bonk::NothingType* type) {
    stream.get_stream() << "nothing";
}

void bonk::TypePrinter::visit(const bonk::NeverType* type) {
    stream.get_stream() << "never";
}

void bonk::TypePrinter::visit(const bonk::ErrorType* type) {
    stream.get_stream() << "error type";
}

void bonk::FootprintCounter::visit(const bonk::HiveType* type) {
    footprint = pointer_size;
}

void bonk::FootprintCounter::visit(const bonk::BlokType* type) {
    footprint = pointer_size;
}

void bonk::FootprintCounter::visit(const bonk::TrivialType* type) {
    switch (type->primitive_type) {
    case PrimitiveType::t_unset:
        footprint = 0;
        return;
    case PrimitiveType::t_buul:
        footprint = 1;
        return;
    case PrimitiveType::t_shrt:
        footprint = 2;
        return;
    case PrimitiveType::t_nubr:
    case PrimitiveType::t_flot:
        footprint = 4;
        return;
    case PrimitiveType::t_long:
    case PrimitiveType::t_dabl:
        footprint = 8;
        return;
    case PrimitiveType::t_strg:
        footprint = pointer_size;
        return;
    }
    footprint = 0;
}

void bonk::FootprintCounter::visit(const bonk::ManyType* type) {
    footprint = pointer_size;
}

void bonk::FootprintCounter::visit(const bonk::NothingType* type) {
    footprint = 0;
}

void bonk::FootprintCounter::visit(const bonk::NeverType* type) {
    footprint = 0;
}

void bonk::FootprintCounter::visit(const bonk::ErrorType* type) {
    footprint = 0;
}

int bonk::FootprintCounter::get_footprint(bonk::Type* type) {
    FootprintCounter counter;
    type->accept(&counter);
    return counter.footprint;
}

void bonk::NeverSearchVisitor::visit(const bonk::HiveType* type) {
    result = false;
}

void bonk::NeverSearchVisitor::visit(const bonk::BlokType* type) {
    type->return_type->accept(this);
}

void bonk::NeverSearchVisitor::visit(const bonk::TrivialType* type) {
    result = false;
}

void bonk::NeverSearchVisitor::visit(const bonk::ManyType* type) {
    type->element_type->accept(this);
}

void bonk::NeverSearchVisitor::visit(const bonk::NothingType* type) {
    result = false;
}

void bonk::NeverSearchVisitor::visit(const bonk::NeverType* type) {
    result = true;
}

void bonk::NeverSearchVisitor::visit(const bonk::ErrorType* type) {
    result = false;
}

bool bonk::NeverSearchVisitor::search(bonk::Type* type) {
    NeverSearchVisitor visitor;
    type->accept(&visitor);
    return visitor.result;
}