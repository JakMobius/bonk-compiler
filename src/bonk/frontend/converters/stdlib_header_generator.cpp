
#include "stdlib_header_generator.hpp"
#include "bonk/frontend/ast/ast_clone_visitor.hpp"

bonk::AST bonk::StdLibHeaderGenerator::generate() {

    bonk::AST result;
    current_ast = &result;
    result.root = std::make_unique<TreeNodeProgram>();
    auto program = result.root.get();

    generate_stdlib_function("$$bonk_create_object")
        .parameter("size", TrivialTypeKind::t_nubr)
        .return_type(TrivialTypeKind::t_long)
        .attach(program);

    generate_stdlib_function("$$bonk_object_free")
        .parameter("object", TrivialTypeKind::t_long)
        .return_type(TrivialTypeKind::t_nubr)
        .attach(program);

    return result;
}

bonk::StdlibFunction bonk::StdLibHeaderGenerator::generate_stdlib_function(std::string_view name) {

    std::string_view symbol = current_ast->buffer.get_symbol(std::string(name));
    auto block_definition = std::make_unique<TreeNodeBlockDefinition>();
    block_definition->block_name = std::make_unique<TreeNodeIdentifier>();
    block_definition->block_name->identifier_text = symbol;
    block_definition->block_parameters = std::make_unique<TreeNodeParameterListDefinition>();

    return {*this, std::move(block_definition)};
}

bonk::StdlibFunction& bonk::StdlibFunction::parameter(std::string_view name,
                                                      bonk::TrivialTypeKind type) {
    auto parameter = std::make_unique<TreeNodeVariableDefinition>();

    parameter->variable_name = std::make_unique<TreeNodeIdentifier>();
    parameter->variable_name->identifier_text =
        generator.current_ast->buffer.get_symbol(std::string(name));

    auto parameter_type = std::make_unique<TreeNodePrimitiveType>();
    parameter_type->primitive_type = type;

    parameter->variable_type = std::move(parameter_type);

    function->block_parameters->parameters.push_back(std::move(parameter));

    return *this;
}

bonk::StdlibFunction& bonk::StdlibFunction::return_type(bonk::TrivialTypeKind return_type) {
    auto trivial_type = std::make_unique<TrivialType>();
    trivial_type->trivial_kind = return_type;

    auto type = generator.front_end.type_table.annotate<BlokType>(function.get());
    type->return_type = std::move(trivial_type);

    for (auto& parameter : function->block_parameters->parameters) {
        type->parameters.push_back(parameter.get());
    }

    return *this;
}

void bonk::StdlibFunction::attach(bonk::TreeNodeProgram* program) {
    program->body.push_front(std::move(function));
}
