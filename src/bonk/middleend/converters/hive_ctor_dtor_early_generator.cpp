
#include "hive_ctor_dtor_early_generator.hpp"
#include "bonk/tree/ast_clone_visitor.hpp"

void bonk::HiveConstructorDestructorEarlyGenerator::generate(bonk::TreeNode* ast) {
    assert(ast->type == TreeNodeType::n_program);

    auto program = (TreeNodeProgram*)ast;

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
    constructor->block_name->identifier_text =
        middle_end.hidden_text_storage.get_hidden_symbol(constructor_name);

    // Trick the type-checker into thinking that the constructor is already checked
    // and has the return type of hive

    auto type = middle_end.type_table.annotate<BlokType>(constructor.get());
    auto return_type = std::make_unique<HiveType>();
    return_type->hive_definition = hive_definition;
    type->return_type = std::move(return_type);

    // Generate the parameter list
    constructor->block_parameters = std::make_unique<TreeNodeParameterListDefinition>();

    for (auto& field : hive_fields) {
        constructor->block_parameters->parameters.push_back(ASTCloneVisitor().clone(field));
        type->parameters.push_back(field);
    }

    hive_fields.clear();

    // Generate the body
    constructor->body = std::make_unique<TreeNodeCodeBlock>();

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
    destructor->block_name->identifier_text =
        middle_end.hidden_text_storage.get_hidden_symbol(destructor_name);

    // Generate the parameter list. The only parameter is the hive itself
    destructor->block_parameters = std::make_unique<TreeNodeParameterListDefinition>();

    auto hive_parameter = std::make_unique<TreeNodeVariableDefinition>();
    hive_parameter->variable_name = std::make_unique<TreeNodeIdentifier>();
    hive_parameter->variable_name->identifier_text =
        middle_end.hidden_text_storage.get_hidden_symbol("object");

    auto hive_type_identifier = std::make_unique<TreeNodeIdentifier>();
    hive_type_identifier->identifier_text = hive_definition->hive_name->identifier_text;

    hive_parameter->variable_type = std::move(hive_type_identifier);

    destructor->block_parameters->parameters.push_back(std::move(hive_parameter));

    destructor->body = std::make_unique<TreeNodeCodeBlock>();

    return destructor;
}