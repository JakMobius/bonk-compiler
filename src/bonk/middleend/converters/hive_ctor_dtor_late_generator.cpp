
#include "hive_ctor_dtor_late_generator.hpp"
#include "bonk/middleend/annotators/basic_symbol_annotator.hpp"
#include "bonk/middleend/annotators/type_annotator.hpp"
#include "bonk/tree/ast_clone_visitor.hpp"

void bonk::HiveConstructorDestructorLateGenerator::generate(bonk::TreeNode* ast) {
    ast->accept(this);
}

void bonk::HiveConstructorDestructorLateGenerator::visit(bonk::TreeNodeHiveDefinition* node) {
    // Do not visit hive definition to save time

    std::string_view hive_name = node->hive_name->identifier_text;

    std::string ctor_name = std::string(hive_name) + "$$constructor";
    std::string dtor_name = std::string(hive_name) + "$$destructor";

    // Get the hive scope
    auto scope = middle_end.symbol_table.symbol_scopes[node];

    ScopedNameResolver resolver;
    resolver.current_scope = scope;

    // Find constructor and destructor definitions and fill them up
    auto* ctor_definition = (TreeNodeBlockDefinition*)resolver.get_name_definition(ctor_name);
    auto* dtor_definition = (TreeNodeBlockDefinition*)resolver.get_name_definition(dtor_name);

    fill_constructor(ctor_definition, node);
    fill_destructor(dtor_definition, node);
}

void bonk::HiveConstructorDestructorLateGenerator::visit(bonk::TreeNodeBlockDefinition* node) {
    // Do not visit block definition to save time
}

void bonk::HiveConstructorDestructorLateGenerator::fill_constructor(
    bonk::TreeNodeBlockDefinition* constructor, bonk::TreeNodeHiveDefinition* hive) {

    // The only thing we need to do here is to find the size argument of $$bonk_create_object
    // and replace it with the actual size of the hive

    // Generate 'bowl object = @$$bonk_create_object[size = ...]'

    // First, call the library function to construct the hive
    auto call = std::make_unique<TreeNodeCall>();
    auto callee_identifier = std::make_unique<TreeNodeIdentifier>();
    callee_identifier->identifier_text =
        middle_end.hidden_text_storage.get_hidden_symbol("$$bonk_create_object");
    call->callee = std::move(callee_identifier);
    call->arguments = std::make_unique<TreeNodeParameterList>();

    auto size_parameter_name = std::make_unique<TreeNodeIdentifier>();
    size_parameter_name->identifier_text = middle_end.hidden_text_storage.get_hidden_symbol("size");

    int hive_size = middle_end.get_hive_field_offset(hive, -1);

    auto size_placeholder_name = std::make_unique<TreeNodeNumberConstant>();
    size_placeholder_name->contents.set_integer(hive_size);

    auto size_parameter = std::make_unique<TreeNodeParameterListItem>();
    size_parameter->parameter_value = std::move(size_placeholder_name);
    size_parameter->parameter_name = std::move(size_parameter_name);

    call->arguments->parameters.push_back(std::move(size_parameter));

    // Annotate the call with the hive type to trick the type checker
    // into thinking that the call returns the hive type instead of
    // a number. This is kind of a hack, but it works.
    middle_end.type_table.annotate<HiveType>(call.get())->hive_definition = hive;

    auto object_name = middle_end.hidden_text_storage.get_hidden_symbol("object");

    auto object_variable_definition = std::make_unique<TreeNodeVariableDefinition>();
    object_variable_definition->variable_name = std::make_unique<TreeNodeIdentifier>();
    object_variable_definition->variable_name->identifier_text = object_name;
    object_variable_definition->variable_value = std::move(call);

    constructor->body->body.push_back(std::move(object_variable_definition));

    for (auto& field : constructor->block_parameters->parameters) {
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
        assignment->right = ASTCloneVisitor().clone(field->variable_name.get());
        assignment->operator_type = OperatorType::o_assign;
        constructor->body->body.push_back(std::move(assignment));
    }

    // Generate 'bonk object'
    auto bonk = std::make_unique<TreeNodeBonkStatement>();
    auto object_identifier = std::make_unique<TreeNodeIdentifier>();
    object_identifier->identifier_text = object_name;
    bonk->expression = std::move(object_identifier);
    constructor->body->body.push_back(std::move(bonk));

    // Now, the constructor body should be annotated properly

    BasicSymbolAnnotator symbol_annotator{middle_end};
    symbol_annotator.scoped_name_resolver.current_scope =
        middle_end.symbol_table.symbol_scopes[hive]->parent_scope;
    constructor->accept(&symbol_annotator);

    TypeAnnotator annotator{middle_end};
    annotator.annotate_ast(constructor);
}

void bonk::HiveConstructorDestructorLateGenerator::fill_destructor(
    bonk::TreeNodeBlockDefinition* destructor, bonk::TreeNodeHiveDefinition* hive) {

    // Find the hive fields that are hives and set their pointers to null
    // Since the language doesn't have null keyword, we will use the
    // '0' number constant and cast it to the hive type with use of the
    // type table.

    for (auto& field : hive->body) {
        if (field->type != TreeNodeType::n_variable_definition)
            continue;
        auto variable = (TreeNodeVariableDefinition*)field.get();

        auto type = middle_end.type_table.get_type(variable->variable_type.get());
        if (type->kind != TypeKind::hive)
            continue;

        // Generate 'field of object = 0'

        auto field_identifier = std::make_unique<TreeNodeIdentifier>();
        field_identifier->identifier_text = variable->variable_name->identifier_text;

        auto object_identifier = std::make_unique<TreeNodeIdentifier>();
        object_identifier->identifier_text =
            middle_end.hidden_text_storage.get_hidden_symbol("object");

        auto object_access = std::make_unique<TreeNodeHiveAccess>();
        object_access->hive = std::move(object_identifier);
        object_access->field = std::move(field_identifier);

        auto assignment = std::make_unique<TreeNodeBinaryOperation>();
        assignment->left = std::move(object_access);
        assignment->right = std::make_unique<TreeNodeNumberConstant>();
        assignment->operator_type = OperatorType::o_assign;

        // Cast the number constant to the hive type
        auto hive_type = middle_end.type_table.annotate<HiveType>(assignment->right.get());
        hive_type->hive_definition = ((HiveType*)type)->hive_definition;

        destructor->body->body.push_back(std::move(assignment));
    }

    // Now, the destructor body should be annotated properly

    BasicSymbolAnnotator symbol_annotator{middle_end};
    symbol_annotator.scoped_name_resolver.current_scope =
        middle_end.symbol_table.symbol_scopes[hive]->parent_scope;
    destructor->accept(&symbol_annotator);

    TypeAnnotator annotator{middle_end};
    annotator.annotate_ast(destructor);
}
