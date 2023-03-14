
#include "stdlib_header_generator.hpp"
#include "bonk/tree/ast_clone_visitor.hpp"

void bonk::StdLibHeaderGenerator::generate(bonk::TreeNode* ast) {
    assert(ast->type == TreeNodeType::n_program);

    auto program = (TreeNodeProgram*)ast;

    generate_stdlib_function("$$bonk_create_object")
        .parameter("size", PrimitiveType::t_nubr)
        .return_type(PrimitiveType::t_long)
        .attach(program);

    generate_stdlib_function("$$bonk_object_free")
        .parameter("object", PrimitiveType::t_long)
        .return_type(PrimitiveType::t_nubr)
        .attach(program);
}

bonk::StdlibFunction bonk::StdLibHeaderGenerator::generate_stdlib_function(std::string_view name) {

    std::string_view symbol = middle_end.hidden_text_storage.get_hidden_symbol(std::string(name));
    auto block_definition = std::make_unique<TreeNodeBlockDefinition>();
    block_definition->block_name = std::make_unique<TreeNodeIdentifier>();
    block_definition->block_name->identifier_text = symbol;
    block_definition->block_parameters = std::make_unique<TreeNodeParameterListDefinition>();

    return {middle_end, std::move(block_definition)};
}

bonk::StdlibFunction& bonk::StdlibFunction::parameter(std::string_view name,
                                                      bonk::PrimitiveType type) {
    auto parameter = std::make_unique<TreeNodeVariableDefinition>();

    parameter->variable_name = std::make_unique<TreeNodeIdentifier>();
    parameter->variable_name->identifier_text =
        middle_end.hidden_text_storage.get_hidden_symbol(std::string(name));

    auto parameter_type = std::make_unique<TreeNodePrimitiveType>();
    parameter_type->primitive_type = type;

    parameter->variable_type = std::move(parameter_type);

    function->block_parameters->parameters.push_back(std::move(parameter));

    return *this;
}

bonk::StdlibFunction& bonk::StdlibFunction::return_type(bonk::PrimitiveType return_type) {
    auto trivial_type = std::make_unique<TrivialType>();
    trivial_type->primitive_type = return_type;

    auto type = middle_end.type_table.annotate<BlokType>(function.get());
    type->return_type = std::move(trivial_type);

    for(auto& parameter : function->block_parameters->parameters) {
        type->parameters.push_back(parameter.get());
    }

    return *this;
}

void bonk::StdlibFunction::attach(bonk::TreeNodeProgram* program) {
    program->body.push_front(std::move(function));
}
