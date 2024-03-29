
#include "basic_symbol_annotator.hpp"
#include <sstream>

bonk::BasicSymbolAnnotator::BasicSymbolAnnotator(bonk::FrontEnd& frontend)
    : frontend(frontend) {
    scoped_name_resolver.current_scope = frontend.symbol_table.global_scope;
}

bool bonk::BasicSymbolAnnotator::annotate_ast(bonk::AST& ast) {
    errors_occurred = false;
    ast.root->accept(this);
    return !errors_occurred;
}

void bonk::BasicSymbolAnnotator::visit(bonk::TreeNodeProgram* node) {
    ForwardDeclaringSymbolAnnotator(*this).visit(node);
    ASTVisitor::visit(node);
}

void bonk::BasicSymbolAnnotator::visit(bonk::TreeNodeCodeBlock* node) {
    push_scope(node);
    ASTVisitor::visit(node);
    pop_scope();
}

void bonk::BasicSymbolAnnotator::visit(bonk::TreeNodeHiveDefinition* node) {
    push_scope(node);
    ForwardDeclaringSymbolAnnotator(*this).visit(node);
    ASTVisitor::visit(node);
    pop_scope();
}

void bonk::BasicSymbolAnnotator::visit(bonk::TreeNodeVariableDefinition* node) {
    // Only handle variable definitions that are not in a hive,
    // because others are handled by the ForwardDeclaringSymbolAnnotator
    if (scoped_name_resolver.current_scope->get_type() != TreeNodeType::n_hive_definition) {
        handle_definition(node);
    }

    ASTVisitor::visit(node);
}

void bonk::BasicSymbolAnnotator::visit(bonk::TreeNodeBlockDefinition* node) {
    // Do not call handle_definition here, because block definitions
    // are handled by the ForwardDeclaringSymbolAnnotator

    push_scope(node);

    if (node->block_parameters) {
        for (auto& parameter : node->block_parameters->parameters) {
            scoped_name_resolver.define_variable(get_definition_identifier(parameter.get()),
                                                 parameter.get());
            frontend.symbol_table.symbol_definitions[parameter.get()] =
                SymbolDefinition::local(parameter.get());

            // Visit the parameter type, if it exists
            if (parameter->variable_type) {
                parameter->variable_type->accept(this);
            }
        }
    }

    if (node->return_type) {
        node->return_type->accept(this);
    }

    // Do not use ASTVisitor::visit here, because we don't want to visit the parameters
    // again
    if (node->body) {
        node->body->accept(this);
    }
    pop_scope();
}

void bonk::BasicSymbolAnnotator::visit(bonk::TreeNodeLoopStatement* node) {
    push_scope(node);
    ASTVisitor::visit(node);

    if (node->loop_parameters) {
        for (auto& parameter : node->loop_parameters->parameters) {
            scoped_name_resolver.define_variable(get_definition_identifier(parameter.get()),
                                                 parameter.get());
            frontend.symbol_table.symbol_definitions[parameter.get()] = SymbolDefinition::local(parameter.get());
        }
    }
    pop_scope();
}

void bonk::BasicSymbolAnnotator::visit(bonk::TreeNodeParameterListItem* node) {
    // Only accept the parameter value, since parameter name cannot be verified
    // at this stage. (We don't know the callee type yet)
    if (node->parameter_value)
        node->parameter_value->accept(this);
}

void bonk::BasicSymbolAnnotator::visit(bonk::TreeNodeIdentifier* node) {
    frontend.symbol_table.symbol_scopes[node] = scoped_name_resolver.current_scope;
    auto definition = scoped_name_resolver.get_name_definition(node->identifier_text);

    if (definition != nullptr) {
        frontend.symbol_table.symbol_definitions[node] = SymbolDefinition::local(definition);
        frontend.symbol_table.symbol_names[node] = name_for_def_in_current_scope(definition);
        return;
    }

    auto external_symbol_table = frontend.external_symbol_table.external_symbol_def_files;
    auto it = external_symbol_table.find(node);
    if (it != external_symbol_table.end()) {
        std::string_view file = frontend.external_symbol_table.get_external_file(it->second);
        frontend.symbol_table.symbol_definitions[node] = SymbolDefinition::external(file);
        frontend.symbol_table.symbol_names[node] = std::string(node->identifier_text);
        return;
    }

    errors_occurred = true;
    frontend.compiler.error().at(node->source_position)
            << "Identifier '" << node->identifier_text << "' is not defined";
}

void bonk::BasicSymbolAnnotator::handle_definition(bonk::TreeNode* node) {
    auto definition_identifier = get_definition_identifier(node);

    auto it = scoped_name_resolver.current_scope->symbols.find(definition_identifier);

    if (it != scoped_name_resolver.current_scope->symbols.end()) {
        errors_occurred = true;
        frontend.compiler.error().at(node->source_position)
            << "Identifier '" << definition_identifier << "' is already defined in this scope";
        return;
    }

    scoped_name_resolver.define_variable(definition_identifier, node);

    frontend.symbol_table.symbol_definitions[node] = SymbolDefinition::local(node);
    frontend.symbol_table.symbol_names[node] = name_for_def_in_current_scope(node);
}

std::string bonk::BasicSymbolAnnotator::name_for_def_in_current_scope(bonk::TreeNode* node) {
    std::string_view identifier_text = get_definition_identifier(node);

    if (node->type != TreeNodeType::n_block_definition &&
        node->type != TreeNodeType::n_hive_definition) {
        return std::string(identifier_text);
    }

    std::stringstream actual_name;

    actual_name << identifier_text;

    for (auto scope = scoped_name_resolver.current_scope; scope; scope = scope->parent_scope) {
        if (scope->get_type() != TreeNodeType::n_block_definition &&
            scope->get_type() != TreeNodeType::n_hive_definition) {
            continue;
        }

        actual_name << " of " << get_definition_identifier(scope->definition);
    }

    return actual_name.str();
}

std::string_view bonk::BasicSymbolAnnotator::get_definition_identifier(bonk::TreeNode* definition) {
    switch (definition->type) {
    case TreeNodeType::n_variable_definition:
        return ((TreeNodeVariableDefinition*)definition)->variable_name->identifier_text;
    case TreeNodeType::n_block_definition:
        return ((TreeNodeBlockDefinition*)definition)->block_name->identifier_text;
    case TreeNodeType::n_hive_definition:
        return ((TreeNodeHiveDefinition*)definition)->hive_name->identifier_text;
    default:
        assert(!"Invalid definition type");
    }
}
void bonk::BasicSymbolAnnotator::visit(bonk::TreeNodeHiveAccess* node) {
    // Ignore the field name for now, since we don't know the type of the hive yet
    node->hive->accept(this);
}

void bonk::BasicSymbolAnnotator::push_scope(bonk::TreeNode* ast_node) {
    auto new_scope = frontend.symbol_table.create_scope(ast_node, scoped_name_resolver.current_scope);
    scoped_name_resolver.current_scope = new_scope;
}

void bonk::BasicSymbolAnnotator::pop_scope() {
    scoped_name_resolver.current_scope = scoped_name_resolver.current_scope->parent_scope;
}

bonk::ScopedNameResolver::ScopedNameResolver() {
}

bonk::TreeNode* bonk::ScopedNameResolver::get_name_definition(std::string_view name) {
    for (auto scope = current_scope; scope; scope = scope->parent_scope) {
        auto it = scope->symbols.find(name);
        if (it != scope->symbols.end()) {
            return it->second;
        }
    }
    return nullptr;
}

void bonk::ScopedNameResolver::define_variable(std::string_view name, bonk::TreeNode* definition) {
    current_scope->symbols[name] = definition;
}

bonk::ScopedNameResolver::ScopedNameResolver(bonk::SymbolScope* scope) {
    current_scope = scope;
}

bool ForwardDeclaringSymbolAnnotator::visit_guard() {
    // This guard prevents this visitor from going deeper into the AST
    if (!should_visit) {
        return false;
    }
    should_visit = false;
    return true;
}

void ForwardDeclaringSymbolAnnotator::visit(bonk::TreeNodeProgram* node) {
    // visit_guard() is mandatory here, because otherwise the visitor would
    // visit symbols of depth 2. (e.g. blok defined as a member of a hive)
    // TODO: Write a test for this

    if (visit_guard()) {
        ASTVisitor::visit(node);
    }
}

void ForwardDeclaringSymbolAnnotator::visit(bonk::TreeNodeHiveDefinition* node) {
    if (visit_guard()) {
        ASTVisitor::visit(node);
    } else {
        // Forward declare hive definitions, but only on the depth of 1
        annotator.handle_definition(node);
    }
}

void ForwardDeclaringSymbolAnnotator::visit(bonk::TreeNodeBlockDefinition* node) {
    // Forward declare block definitions
    annotator.handle_definition(node);
}

void ForwardDeclaringSymbolAnnotator::visit(bonk::TreeNodeVariableDefinition* node) {

    // Only forward declare fields in hives. Variables in code blocks can not
    // be used before they are defined.
    if (annotator.scoped_name_resolver.current_scope->definition->type ==
        bonk::TreeNodeType::n_hive_definition)
        annotator.handle_definition(node);
}
