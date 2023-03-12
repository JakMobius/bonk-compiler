
#include "hive_constructor_generator.hpp"
#include "bonk/tree/ast_clone_visitor.hpp"

void bonk::HiveConstructorGenerator::generate(bonk::TreeNode* ast) {
    assert(ast->type == TreeNodeType::n_program);

    auto program = (TreeNodeProgram*)ast;

    for (auto it = program->body.begin(); it != program->body.end();) {
        if ((*it)->type != TreeNodeType::n_hive_definition) {
            ++it;
            continue;
        }

        auto constructor = generate_hive_constructor((TreeNodeHiveDefinition*)it->get());

        // Insert constructor after the hive definition
        ++it;
        it = program->body.insert(it, std::move(constructor));
    }
}

std::unique_ptr<bonk::TreeNode>
bonk::HiveConstructorGenerator::generate_hive_constructor(TreeNodeHiveDefinition* hive_definition) {

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

    // Generate the parameter list
    constructor->block_parameters = std::make_unique<TreeNodeParameterListDefinition>();

    ASTCloneVisitor clone_visitor;

    for (auto& field : hive_fields) {
        constructor->block_parameters->parameters.push_back(clone_visitor.clone(field));
    }

    // Generate the body
    constructor->body = std::make_unique<TreeNodeCodeBlock>();

    // Generate 'bowl object = @$$bonk_create_object[size = <x>]'
    // First, call the library function to construct the hive
    auto call = std::make_unique<TreeNodeCall>();
    auto callee_identifier = std::make_unique<TreeNodeIdentifier>();
    callee_identifier->identifier_text =
        middle_end.hidden_text_storage.get_hidden_symbol("$$bonk_create_object");
    call->callee = std::move(callee_identifier);
    call->arguments = std::make_unique<TreeNodeParameterList>();

    int hive_size = get_hive_footprint();

    auto size_parameter_name = std::make_unique<TreeNodeIdentifier>();
    size_parameter_name->identifier_text = middle_end.hidden_text_storage.get_hidden_symbol("size");

    auto size_parameter_constant = std::make_unique<TreeNodeNumberConstant>();
    size_parameter_constant->integer_value = hive_size;
    size_parameter_constant->double_value = hive_size;

    auto size_parameter = std::make_unique<TreeNodeParameterListItem>();
    size_parameter->parameter_value = std::move(size_parameter_constant);
    size_parameter->parameter_name = std::move(size_parameter_name);

    call->arguments->parameters.push_back(std::move(size_parameter));

    // Annotate the call with the hive type to trick the type checker
    // into thinking that the call returns the hive type instead of
    // a number. This is kind of a hack, but it works.
    middle_end.type_table.annotate<HiveType>(call.get())->hive_definition = hive_definition;

    auto object_name = middle_end.hidden_text_storage.get_hidden_symbol("object");

    auto object_variable_definition = std::make_unique<TreeNodeVariableDefinition>();
    object_variable_definition->variable_name = std::make_unique<TreeNodeIdentifier>();
    object_variable_definition->variable_name->identifier_text = object_name;
    object_variable_definition->variable_value = std::move(call);

    constructor->body->body.push_back(std::move(object_variable_definition));

    for (auto& field : hive_fields) {
        // Generate 'field of object = field'

        auto field_identifier = std::make_unique<TreeNodeIdentifier>();
        field_identifier->identifier_text = field->variable_name->identifier_text;

        auto object_identifier = std::make_unique<TreeNodeIdentifier>();
        object_identifier->identifier_text = object_name;

        auto object_access = std::make_unique<TreeNodeHiveAccess>();
        object_access->hive = std::move(object_identifier);
        object_access->field = std::move(field_identifier);

        auto assignment = std::make_unique<TreeNodeBinaryOperation>();
        assignment->left = std::move(object_access);
        assignment->right = clone_visitor.clone(field->variable_name.get());
        assignment->operator_type = OperatorType::o_assign;
        constructor->body->body.push_back(std::move(assignment));
    }

    // Generate 'bonk object'
    auto bonk = std::make_unique<TreeNodeBonkStatement>();
    auto object_identifier = std::make_unique<TreeNodeIdentifier>();
    object_identifier->identifier_text = object_name;
    bonk->expression = std::move(object_identifier);
    constructor->body->body.push_back(std::move(bonk));

    return constructor;
}
int bonk::HiveConstructorGenerator::get_hive_footprint() {
    // TODO: calculate the footprint of the hive
    return 64;
}
