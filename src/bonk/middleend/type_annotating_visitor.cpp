
#include "type_annotating_visitor.hpp"
#include "../compiler.hpp"
#include "type_inferring.hpp"

bonk::ScopedNameResolver::ScopedNameResolver() {
    scopes.emplace_back();
}

void bonk::ScopedNameResolver::push_scope() {
    //    padding();
    //    std::cout << "Pushing scope\n";
    scopes.emplace_back();
}

void bonk::ScopedNameResolver::pop_scope() {
    //    padding();
    //    std::cout << "Popping scope\n";
    scopes.pop_back();
}

bonk::TreeNode* bonk::ScopedNameResolver::get_name_definition(std::string_view name) {
    for (int i = scopes.size() - 1; i >= 0; i--) {
        auto it = scopes[i].variables.find(name);
        if (it != scopes[i].variables.end()) {
            //            padding();
            //            std::cout << "Identifier " << name << " is defined as " << it->second <<
            //            std::endl;
            return name_definitions[it->second];
        }
    }
    return nullptr;
}

void bonk::ScopedNameResolver::padding() {
    for (int i = 0; i < scopes.size(); i++) {
        std::cout << "  ";
    }
}

void bonk::ScopedNameResolver::define_variable(bonk::TreeNode* definition) {
    std::string_view name;

    //    padding();

    switch (definition->type) {
    case TreeNodeType::n_variable_definition: {
        name = ((TreeNodeVariableDefinition*)definition)->variable_name->identifier_text;
        //        std::cout << "Defining variable " << name << " as " << total_names << std::endl;
        break;
    }
    case TreeNodeType::n_block_definition: {
        name = ((TreeNodeBlockDefinition*)definition)->block_name->identifier_text;
        //        std::cout << "Defining block " << name << " as " << total_names << std::endl;
        break;
    }
    case TreeNodeType::n_hive_definition: {
        name = ((TreeNodeHiveDefinition*)definition)->hive_name->identifier_text;
        //        std::cout << "Defining hive " << name << " as " << total_names << std::endl;
        break;
    }
    default: {
        assert(!"Invalid definition type");
    }
    }

    scopes.back().variables[name] = total_names;
    identifier_definitions[definition] = definition;
    name_definitions[total_names] = definition;

    total_names++;
}

bonk::TreeNode* bonk::ScopedNameResolver::get_definition(int name_id) {
    auto it = name_definitions.find(name_id);
    if (it == name_definitions.end())
        return nullptr;
    return it->second;
}

bonk::TypeAnnotatingVisitor::TypeAnnotatingVisitor(bonk::Compiler& linked_compiler)
    : linked_compiler(linked_compiler), active_name_resolver(&scoped_name_resolver) {
}

bonk::Type* bonk::TypeAnnotatingVisitor::infer_type(bonk::TreeNode* node) {
    TypeInferringVisitor visitor{*this};
    return visitor.infer_type(node);
}

void bonk::TypeAnnotatingVisitor::visit(bonk::TreeNodeCodeBlock* node) {
    scoped_name_resolver.push_scope();
    ASTVisitor::visit(node);
    scoped_name_resolver.pop_scope();
}

void bonk::TypeAnnotatingVisitor::visit(bonk::TreeNodeHiveDefinition* node) {
    infer_type(node);

    scoped_name_resolver.define_variable(node);
    scoped_name_resolver.push_scope();
    ASTVisitor::visit(node);
    scoped_name_resolver.pop_scope();
}

void bonk::TypeAnnotatingVisitor::visit(bonk::TreeNodeVariableDefinition* node) {
    infer_type(node);

    scoped_name_resolver.define_variable(node);
}

void bonk::TypeAnnotatingVisitor::visit(bonk::TreeNodeBlockDefinition* node) {
    infer_type(node);

    scoped_name_resolver.define_variable(node);
    scoped_name_resolver.push_scope();

    if (node->block_parameters) {
        for (auto& parameter : node->block_parameters->parameters) {
            scoped_name_resolver.define_variable(parameter.get());
        }
    }

    ASTVisitor::visit(node);
    scoped_name_resolver.pop_scope();
}

void bonk::TypeAnnotatingVisitor::visit(bonk::TreeNodeLoopStatement* node) {
    scoped_name_resolver.push_scope();
    ASTVisitor::visit(node);

    if (node->loop_parameters) {
        for (auto& parameter : node->loop_parameters->parameters) {
            scoped_name_resolver.define_variable(parameter.get());
        }
    }
    scoped_name_resolver.pop_scope();
}

void bonk::TypeAnnotatingVisitor::visit(bonk::TreeNodeHiveAccess* node) {
    infer_type(node);
    if (node->hive) node->hive->accept(this);
    if (node->field) node->field->accept(this);
}

void bonk::TypeAnnotatingVisitor::visit(bonk::TreeNodeCall* node) {
    infer_type(node);
    Type* type = infer_type(node->callee.get());

    if(!type || type->kind == TypeKind::error) return;

    if (node->callee)
        node->callee->accept(this);
    if (node->arguments)
        node->arguments->accept(this);
}

void bonk::TypeAnnotatingVisitor::visit(bonk::TreeNodeParameterListItem* node) {
    if (node->parameter_value)
        node->parameter_value->accept(this);
    if (node->parameter_name) {
        node->parameter_name->accept(this);
    }
}

void bonk::TypeAnnotatingVisitor::visit(TreeNodeArrayConstant* node) {
    ASTVisitor::visit(node);
    infer_type(node);
}

void bonk::TypeAnnotatingVisitor::visit(TreeNodeNumberConstant* node) {
    ASTVisitor::visit(node);
    infer_type(node);
}

void bonk::TypeAnnotatingVisitor::visit(TreeNodeStringConstant* node) {
    ASTVisitor::visit(node);
    infer_type(node);
}

void bonk::TypeAnnotatingVisitor::visit(TreeNodeBinaryOperation* node) {
    ASTVisitor::visit(node);
    infer_type(node);
}

void bonk::TypeAnnotatingVisitor::visit(TreeNodeUnaryOperation* node) {
    ASTVisitor::visit(node);
    infer_type(node);
}

void bonk::TypeAnnotatingVisitor::visit(TreeNodePrimitiveType* node) {
    ASTVisitor::visit(node);
    infer_type(node);
}

void bonk::TypeAnnotatingVisitor::visit(TreeNodeManyType* node) {
    ASTVisitor::visit(node);
    infer_type(node);
}

void bonk::TypeAnnotatingVisitor::visit(TreeNodeBonkStatement* node) {
    ASTVisitor::visit(node);
    infer_type(node);
}

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

std::unique_ptr<bonk::Type> bonk::HiveType::shallow_copy() const {
    auto copy = std::make_unique<HiveType>();
    copy->hive_definition = hive_definition;
    return copy;
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

std::unique_ptr<bonk::Type> bonk::BlokType::shallow_copy() const {
    auto copy = std::make_unique<BlokType>();
    copy->parameters = parameters;
    copy->return_type = return_type->shallow_copy();
    return copy;
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

    case t_unset:
        return false;
    case t_nubr:
    case t_flot:
        return *other_type == *this &&
               (operator_type == OperatorType::o_plus || operator_type == OperatorType::o_minus ||
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
    case t_strg:
        return *other_type == *this &&
               (operator_type == OperatorType::o_plus || operator_type == OperatorType::o_assign ||
                operator_type == OperatorType::o_plus_assign ||
                operator_type == OperatorType::o_equal ||
                operator_type == OperatorType::o_not_equal);
    }
}

void bonk::TrivialType::print(std::ostream& stream) const {
    stream << BONK_PRIMITIVE_TYPE_NAMES[primitive_type];
}

bool bonk::TrivialType::allows_unary_operation(bonk::OperatorType operator_type) const {
    switch (primitive_type) {
    case t_unset:
        return false;
    case t_nubr:
    case t_flot:
        return operator_type == OperatorType::o_plus || operator_type == OperatorType::o_minus;
    case t_strg:
        return false;
    }
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

bonk::HiveFieldNameResolver::HiveFieldNameResolver(TreeNodeHiveDefinition* hive_definition)
    : hive_definition(hive_definition) {
}

bonk::TreeNode* bonk::HiveFieldNameResolver::get_name_definition(std::string_view name) {

    for (auto& field : hive_definition->body) {
        switch (field->type) {
        case TreeNodeType::n_variable_definition: {
            auto variable_definition = (TreeNodeVariableDefinition*)field.get();
            if (variable_definition->variable_name->identifier_text == name) {
                return variable_definition;
            }
            break;
        }
        case TreeNodeType::n_block_definition: {
            auto block_definition = (TreeNodeBlockDefinition*)field.get();
            if (block_definition->block_name->identifier_text == name) {
                return block_definition;
            }
            break;
        }
        default:
            break;
        }
    }

    return nullptr;
}

bonk::FunctionParameterNameResolver::FunctionParameterNameResolver(bonk::BlokType* called_function)
    : called_function(called_function) {
}

bonk::TreeNode* bonk::FunctionParameterNameResolver::get_name_definition(std::string_view name) {

    for (auto& parameter : called_function->parameters) {
        if (parameter->variable_name->identifier_text == name) {
            return parameter;
        }
    }

    return nullptr;
}
