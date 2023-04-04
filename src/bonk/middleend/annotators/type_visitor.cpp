
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

void bonk::ConstTypeVisitor::visit(const bonk::ErrorType* type) {
}

void bonk::ConstTypeVisitor::visit(const bonk::ExternalType* type) {
    if(type->resolved) {
        type->resolved->accept(this);
    }
}

void bonk::ConstTypeVisitor::visit(const bonk::NullType* type) {
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
    copy->trivial_kind = type->trivial_kind;
    result = std::move(copy);
}

void bonk::TypeCloner::visit(const bonk::ManyType* type) {
    auto copy = std::make_unique<ManyType>();
    copy->element_type = clone(type->element_type.get());
    result = std::move(copy);
}

void bonk::TypeCloner::visit(const bonk::ErrorType* type) {
    result = std::make_unique<ErrorType>();
}

void bonk::TypeCloner::visit(const bonk::ExternalType* type) {
    auto copy = std::make_unique<ExternalType>();
    if(type->resolved) {
        copy->resolved = clone(type->resolved.get());
    }
    if(type->resolver) {
        copy->resolver = type->resolver->clone();
    }
    result = std::move(copy);
}

void bonk::TypeCloner::visit(const bonk::NullType* type) {
    result = std::make_unique<NullType>();
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
    stream.get_stream() << BONK_TRIVIAL_TYPE_KIND_NAMES[(int)type->trivial_kind];
}

void bonk::TypePrinter::visit(const bonk::ManyType* type) {
    stream.get_stream() << "many ";
    type->element_type->accept(this);
}

void bonk::TypePrinter::visit(const bonk::ErrorType* type) {
    stream.get_stream() << "error type";
}

void bonk::TypePrinter::visit(const bonk::ExternalType* type) {
    if(type->resolved) {
        type->resolved->accept(this);
    } else {
        stream.get_stream() << "<unresolved external type>";
    }
}

void bonk::TypePrinter::visit(const bonk::NullType* type) {
    stream.get_stream() << "null";
}

void bonk::FootprintCounter::visit(const bonk::HiveType* type) {
    footprint = pointer_size;
}

void bonk::FootprintCounter::visit(const bonk::BlokType* type) {
    footprint = pointer_size;
}

void bonk::FootprintCounter::visit(const bonk::TrivialType* type) {
    switch (type->trivial_kind) {
    case TrivialTypeKind::t_unset:
    case TrivialTypeKind::t_nothing:
    case TrivialTypeKind::t_never:
        footprint = 0;
        return;
    case TrivialTypeKind::t_buul:
        footprint = 1;
        return;
    case TrivialTypeKind::t_shrt:
        footprint = 2;
        return;
    case TrivialTypeKind::t_nubr:
    case TrivialTypeKind::t_flot:
        footprint = 4;
        return;
    case TrivialTypeKind::t_long:
    case TrivialTypeKind::t_dabl:
        footprint = 8;
        return;
    case TrivialTypeKind::t_strg:
        footprint = pointer_size;
        return;
    }
    footprint = 0;
}

void bonk::FootprintCounter::visit(const bonk::ManyType* type) {
    footprint = pointer_size;
}

void bonk::FootprintCounter::visit(const bonk::ErrorType* type) {
    footprint = 0;
}

int bonk::FootprintCounter::get_footprint(bonk::Type* type) {
    FootprintCounter counter;
    type->accept(&counter);
    return counter.footprint;
}

void bonk::FootprintCounter::visit(const bonk::ExternalType* type) {
    type->get_resolved()->accept(this);
}

void bonk::FootprintCounter::visit(const bonk::NullType* type) {
    footprint = pointer_size;
}

void bonk::NeverSearchVisitor::visit(const bonk::HiveType* type) {
    result = false;
}

void bonk::NeverSearchVisitor::visit(const bonk::BlokType* type) {
    type->return_type->accept(this);
}

void bonk::NeverSearchVisitor::visit(const bonk::TrivialType* type) {
    result = type->trivial_kind == TrivialTypeKind::t_never;
}

void bonk::NeverSearchVisitor::visit(const bonk::ManyType* type) {
    type->element_type->accept(this);
}

void bonk::NeverSearchVisitor::visit(const bonk::ErrorType* type) {
    result = false;
}

bool bonk::NeverSearchVisitor::search(bonk::Type* type) {
    NeverSearchVisitor visitor;
    type->accept(&visitor);
    return visitor.result;
}

void bonk::NeverSearchVisitor::visit(const bonk::ExternalType* type) {
    type->get_resolved()->accept(this);
}

void bonk::NeverSearchVisitor::visit(const bonk::NullType* type) {
    result = false;
}

void bonk::TypeToASTConvertVisitor::visit(const bonk::HiveType* type) {
    result = ASTCloneVisitor().clone(type->hive_definition->hive_name.get());
}

void bonk::TypeToASTConvertVisitor::visit(const bonk::BlokType* type) {
    assert(false && "BlokType cannot be converted to AST");
}

void bonk::TypeToASTConvertVisitor::visit(const bonk::TrivialType* type) {
    auto node = std::make_unique<TreeNodePrimitiveType>();
    node->primitive_type = type->trivial_kind;
    result = std::move(node);
}

void bonk::TypeToASTConvertVisitor::visit(const bonk::ManyType* type) {
    auto node = std::unique_ptr<TreeNodeManyType>();
    node->parameter = convert(type->element_type.get());
    result = std::move(node);
}

void bonk::TypeToASTConvertVisitor::visit(const bonk::ErrorType* type) {
    assert(false && "ErrorType cannot be converted to AST");
}

std::unique_ptr<bonk::TreeNode> bonk::TypeToASTConvertVisitor::convert(bonk::Type* type) {
    type->accept(this);
    return std::move(result);
}

void bonk::TypeToASTConvertVisitor::visit(const bonk::ExternalType* type) {
    type->get_resolved()->accept(this);
}

void bonk::TypeToASTConvertVisitor::visit(const bonk::NullType* type) {
    auto node = std::make_unique<TreeNodeNull>();
    result = std::move(node);
}
