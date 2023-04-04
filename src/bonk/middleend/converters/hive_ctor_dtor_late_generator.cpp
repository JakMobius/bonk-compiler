
#include "hive_ctor_dtor_late_generator.hpp"
#include "bonk/middleend/annotators/basic_symbol_annotator.hpp"
#include "bonk/middleend/annotators/type_annotator.hpp"
#include "bonk/tree/ast_clone_visitor.hpp"

void bonk::HiveConstructorDestructorLateGenerator::generate(bonk::AST& ast) {
    current_ast = &ast;
    ast.root->accept(this);
    current_ast = nullptr;
}

void bonk::HiveConstructorDestructorLateGenerator::visit(bonk::TreeNodeHiveDefinition* node) {
    // Do not visit hive definition to save time

    std::string_view hive_name = node->hive_name->identifier_text;

    std::string ctor_name = std::string(hive_name) + "$$constructor";
    std::string dtor_name = std::string(hive_name) + "$$destructor";

    // Get the hive scope
    auto scope = middle_end.symbol_table.get_scope_for_node(node);

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

    constructor->body = std::make_unique<TreeNodeCodeBlock>();

    // Generate 'bowl object = @$$bonk_create_object[size = ...]'

    // First, call the library function to construct the hive
    auto call = std::make_unique<TreeNodeCall>();
    auto callee_identifier = std::make_unique<TreeNodeIdentifier>();
    callee_identifier->identifier_text =
        current_ast->buffer.get_symbol("$$bonk_create_object");
    call->callee = std::move(callee_identifier);
    call->arguments = std::make_unique<TreeNodeParameterList>();

    auto size_parameter_name = std::make_unique<TreeNodeIdentifier>();
    size_parameter_name->identifier_text = current_ast->buffer.get_symbol("size");

    int hive_size = middle_end.get_hive_field_offset(hive, -1);

    auto size_placeholder_name = std::make_unique<TreeNodeNumberConstant>();
    size_placeholder_name->contents.set_integer(hive_size);

    auto size_parameter = std::make_unique<TreeNodeParameterListItem>();
    size_parameter->parameter_value = std::move(size_placeholder_name);
    size_parameter->parameter_name = std::move(size_parameter_name);

    call->arguments->parameters.push_back(std::move(size_parameter));

    // Cast the call to the hive type
    auto cast = std::make_unique<TreeNodeCast>();
    cast->operand = std::move(call);

    auto hive_type = std::make_unique<TreeNodeIdentifier>();
    hive_type->identifier_text = hive->hive_name->identifier_text;

    cast->target_type = std::move(hive_type);

    auto object_name = current_ast->buffer.get_symbol("object");

    auto object_variable_definition = std::make_unique<TreeNodeVariableDefinition>();
    object_variable_definition->variable_name = std::make_unique<TreeNodeIdentifier>();
    object_variable_definition->variable_name->identifier_text = object_name;
    object_variable_definition->variable_value = std::move(cast);

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
        middle_end.symbol_table.get_scope_for_node(hive)->parent_scope;
    constructor->accept(&symbol_annotator);

    TypeAnnotator annotator(middle_end);
    constructor->accept(&annotator);
}

void bonk::HiveConstructorDestructorLateGenerator::fill_destructor(
    bonk::TreeNodeBlockDefinition* destructor, bonk::TreeNodeHiveDefinition* hive) {

    destructor->body = std::make_unique<TreeNodeCodeBlock>();

    // Find the hive fields that are hives and set their pointers to null
    // Since the language doesn't have null keyword, we will use the
    // '0' number constant and cast it to the hive type with use of the
    // type table.

    for (auto& field : hive->body) {
        if (field->type != TreeNodeType::n_variable_definition)
            continue;
        auto variable = (TreeNodeVariableDefinition*)field.get();

        auto type = middle_end.type_table.get_type(variable);
        if (type->kind != TypeKind::hive)
            continue;

        // Generate 'field of object = null'

        auto field_identifier = std::make_unique<TreeNodeIdentifier>();
        field_identifier->identifier_text = variable->variable_name->identifier_text;

        auto object_identifier = std::make_unique<TreeNodeIdentifier>();
        object_identifier->identifier_text = current_ast->buffer.get_symbol("object");

        auto object_access = std::make_unique<TreeNodeHiveAccess>();
        object_access->hive = std::move(object_identifier);
        object_access->field = std::move(field_identifier);

        auto assignment = std::make_unique<TreeNodeBinaryOperation>();
        assignment->left = std::move(object_access);
        assignment->right = std::make_unique<TreeNodeNull>();
        assignment->operator_type = OperatorType::o_assign;

        auto cast = std::make_unique<TreeNodeCast>();
        destructor->body->body.push_back(std::move(assignment));
    }

    // Now call the $$bonk_destroy_object function

    // @$$bonk_object_free[object = object]
    auto call = std::make_unique<TreeNodeCall>();
    auto callee_identifier = std::make_unique<TreeNodeIdentifier>();
    callee_identifier->identifier_text =
        current_ast->buffer.get_symbol("$$bonk_object_free");
    call->callee = std::move(callee_identifier);
    call->arguments = std::make_unique<TreeNodeParameterList>();

    auto object_text = current_ast->buffer.get_symbol("object");

    auto object_parameter_name = std::make_unique<TreeNodeIdentifier>();
    object_parameter_name->identifier_text = object_text;

    auto object_parameter_value = std::make_unique<TreeNodeIdentifier>();
    object_parameter_value->identifier_text = object_text;

    // Cast the object to the long type
    auto cast = std::make_unique<TreeNodeCast>();
    auto long_type = std::make_unique<TreeNodePrimitiveType>();
    long_type->primitive_type = TrivialTypeKind::t_long;
    cast->target_type = std::move(long_type);
    cast->operand = std::move(object_parameter_value);

    auto size_parameter = std::make_unique<TreeNodeParameterListItem>();
    size_parameter->parameter_name = std::move(object_parameter_name);
    size_parameter->parameter_value = std::move(cast);

    call->arguments->parameters.push_back(std::move(size_parameter));

    destructor->body->body.push_back(std::move(call));

    // Now, the destructor body should be annotated properly

    BasicSymbolAnnotator symbol_annotator{middle_end};
    symbol_annotator.scoped_name_resolver.current_scope =
        middle_end.symbol_table.get_scope_for_node(hive)->parent_scope;
    destructor->accept(&symbol_annotator);

    TypeAnnotator annotator{middle_end};
    destructor->accept(&annotator);
}
