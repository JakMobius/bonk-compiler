
#include "hive_ctor_dtor_early_generator.hpp"
#include "bonk/tree/ast_clone_visitor.hpp"

void bonk::HiveConstructorDestructorEarlyGenerator::generate(bonk::AST& ast) {
    auto program = ast.root.get();
    current_ast = &ast;

    for (auto it = program->body.begin(); it != program->body.end();) {
        if ((*it)->type != TreeNodeType::n_hive_definition) {
            ++it;
            continue;
        }

        auto constructor = generate_hive_constructor((TreeNodeHiveDefinition*)it->get());
        auto destructor = generate_hive_destructor((TreeNodeHiveDefinition*)it->get());

        // Insert constructor and destructor after the hive definition
        ++it;
        it = program->body.insert(it, std::move(destructor));
        it = program->body.insert(it, std::move(constructor));
    }

    current_ast = nullptr;
}

std::unique_ptr<bonk::TreeNode>
bonk::HiveConstructorDestructorEarlyGenerator::generate_hive_constructor(
    TreeNodeHiveDefinition* hive_definition) {

    for (auto& field : hive_definition->body) {
        if (field->type != TreeNodeType::n_variable_definition) {
            continue;
        }

        hive_fields.push_back((TreeNodeVariableDefinition*)field.get());
    }

    // Generate the constructor
    auto constructor = std::make_unique<TreeNodeBlockDefinition>();

    std::string constructor_name{hive_definition->hive_name->identifier_text};
    constructor_name += "$$constructor";

    // Generate a symbol for the constructor
    constructor->block_name = std::make_unique<TreeNodeIdentifier>();
    constructor->block_name->identifier_text = current_ast->buffer.get_symbol(constructor_name);

    // Generate the parameter list
    constructor->block_parameters = std::make_unique<TreeNodeParameterListDefinition>();

    for (auto& field : hive_fields) {
        constructor->block_parameters->parameters.push_back(ASTCloneVisitor().clone(field));
    }

    hive_fields.clear();

    // Generate the return type
    auto return_type_identifier = std::make_unique<TreeNodeIdentifier>();
    return_type_identifier->identifier_text = hive_definition->hive_name->identifier_text;

    constructor->return_type = std::move(return_type_identifier);

    return constructor;
}

std::unique_ptr<bonk::TreeNode>
bonk::HiveConstructorDestructorEarlyGenerator::generate_hive_destructor(
    TreeNodeHiveDefinition* hive_definition) {
    // Generate the constructor
    auto destructor = std::make_unique<TreeNodeBlockDefinition>();

    std::string destructor_name{hive_definition->hive_name->identifier_text};
    destructor_name += "$$destructor";

    // Generate a symbol for the constructor
    destructor->block_name = std::make_unique<TreeNodeIdentifier>();
    destructor->block_name->identifier_text = current_ast->buffer.get_symbol(destructor_name);

    // Generate the parameter list. The only parameter is the hive itself
    destructor->block_parameters = std::make_unique<TreeNodeParameterListDefinition>();

    auto hive_parameter = std::make_unique<TreeNodeVariableDefinition>();
    hive_parameter->variable_name = std::make_unique<TreeNodeIdentifier>();
    hive_parameter->variable_name->identifier_text = current_ast->buffer.get_symbol("object");

    auto hive_type_identifier = std::make_unique<TreeNodeIdentifier>();
    hive_type_identifier->identifier_text = hive_definition->hive_name->identifier_text;

    hive_parameter->variable_type = std::move(hive_type_identifier);

    destructor->block_parameters->parameters.push_back(std::move(hive_parameter));

    // Generate the return type
    auto return_type = std::make_unique<TreeNodePrimitiveType>();
    return_type->primitive_type = TrivialTypeKind::t_nothing;

    destructor->return_type = std::move(return_type);

    return destructor;
}