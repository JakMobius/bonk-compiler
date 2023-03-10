
#include "basic_symbol_annotator.hpp"
#include <sstream>
#include "../middleend.hpp"

bonk::BasicSymbolAnnotator::BasicSymbolAnnotator(bonk::MiddleEnd& middleend)
    : middleend(middleend) {
}

void bonk::BasicSymbolAnnotator::annotate_ast(bonk::TreeNode* ast) {
    ast->accept(this);
}

void bonk::BasicSymbolAnnotator::visit(bonk::TreeNodeProgram* node) {
    scoped_name_resolver.push_scope(node);
    ForwardDeclaringSymbolAnnotator(*this).visit(node);
    ASTVisitor::visit(node);
    scoped_name_resolver.pop_scope();
}

void bonk::BasicSymbolAnnotator::visit(bonk::TreeNodeCodeBlock* node) {
    scoped_name_resolver.push_scope(node);
    ASTVisitor::visit(node);
    scoped_name_resolver.pop_scope();
}

void bonk::BasicSymbolAnnotator::visit(bonk::TreeNodeHiveDefinition* node) {
    scoped_name_resolver.push_scope(node);
    ForwardDeclaringSymbolAnnotator(*this).visit(node);
    ASTVisitor::visit(node);
    scoped_name_resolver.pop_scope();
}

void bonk::BasicSymbolAnnotator::visit(bonk::TreeNodeVariableDefinition* node) {
    // Only handle variable definitions that are not in a hive,
    // because others are handled by the ForwardDeclaringSymbolAnnotator
    if (scoped_name_resolver.scopes.back().ast_node->type != TreeNodeType::n_hive_definition) {
        handle_definition(node);
    }

    ASTVisitor::visit(node);
}

void bonk::BasicSymbolAnnotator::visit(bonk::TreeNodeBlockDefinition* node) {
    // Do not call handle_definition here, because block definitions
    // are handled by the ForwardDeclaringSymbolAnnotator

    scoped_name_resolver.push_scope(node);

    if (node->block_parameters) {
        for (auto& parameter : node->block_parameters->parameters) {
            scoped_name_resolver.define_variable(get_definition_identifier(parameter.get()),
                                                 parameter.get());
            middleend.symbol_table.symbol_definitions[parameter.get()] = parameter.get();
        }
    }

    // Do not use ASTVisitor::visit here, because we don't want to visit the parameters
    // again
    node->body->accept(this);
    scoped_name_resolver.pop_scope();
}

void bonk::BasicSymbolAnnotator::visit(bonk::TreeNodeLoopStatement* node) {
    scoped_name_resolver.push_scope(node);
    ASTVisitor::visit(node);

    if (node->loop_parameters) {
        for (auto& parameter : node->loop_parameters->parameters) {
            scoped_name_resolver.define_variable(get_definition_identifier(parameter.get()),
                                                 parameter.get());
            middleend.symbol_table.symbol_definitions[parameter.get()] = parameter.get();
        }
    }
    scoped_name_resolver.pop_scope();
}

void bonk::BasicSymbolAnnotator::visit(bonk::TreeNodeParameterListItem* node) {
    // Only accept the parameter value, since parameter name cannot be verified
    // at this stage. (We don't know the callee type yet)
    if (node->parameter_value)
        node->parameter_value->accept(this);
}

void bonk::BasicSymbolAnnotator::visit(bonk::TreeNodeIdentifier* node) {
    auto definition = scoped_name_resolver.get_name_definition(node->identifier_text);
    if (!definition) {
        middleend.linked_compiler.error().at(node->source_position)
            << "Identifier '" << node->identifier_text << "' is not defined";
        return;
    }

    middleend.symbol_table.symbol_definitions[node] = definition;
    middleend.symbol_table.symbol_names[node] = name_for_def_in_current_scope(definition);
}

void bonk::BasicSymbolAnnotator::handle_definition(bonk::TreeNode* node) {
    auto definition_identifier = get_definition_identifier(node);

    auto it = scoped_name_resolver.scopes.back().variables.find(definition_identifier);

    if (it != scoped_name_resolver.scopes.back().variables.end()) {
        middleend.linked_compiler.error().at(node->source_position)
            << "Identifier '" << definition_identifier << "' is already defined in this scope";
        return;
    }

    scoped_name_resolver.define_variable(definition_identifier, node);

    middleend.symbol_table.symbol_definitions[node] = node;
    middleend.symbol_table.symbol_names[node] = name_for_def_in_current_scope(node);
}

std::string bonk::BasicSymbolAnnotator::name_for_def_in_current_scope(bonk::TreeNode* node) {
    std::string_view identifier_text = get_definition_identifier(node);

    if (node->type != TreeNodeType::n_block_definition &&
        node->type != TreeNodeType::n_hive_definition) {
        return std::string(identifier_text);
    }

    std::stringstream actual_name;

    actual_name << identifier_text;

    for (int i = scoped_name_resolver.scopes.size() - 1; i >= 0; i--) {
        auto& scope = scoped_name_resolver.scopes[i];

        if (scope.ast_node->type != TreeNodeType::n_block_definition &&
            scope.ast_node->type != TreeNodeType::n_hive_definition) {
            continue;
        }

        actual_name << " of " << get_definition_identifier(scope.ast_node);
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

bonk::ScopedNameResolver::ScopedNameResolver() {
}

void bonk::ScopedNameResolver::push_scope(TreeNode* ast_node) {
    scopes.emplace_back(ast_node);
}

void bonk::ScopedNameResolver::pop_scope() {
    scopes.pop_back();
}

bonk::TreeNode* bonk::ScopedNameResolver::get_name_definition(std::string_view name) {
    for (int i = scopes.size() - 1; i >= 0; i--) {
        auto it = scopes[i].variables.find(name);
        if (it != scopes[i].variables.end()) {
            return it->second;
        }
    }
    return nullptr;
}

void bonk::ScopedNameResolver::define_variable(std::string_view name, bonk::TreeNode* definition) {
    scopes.back().variables[name] = definition;
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
    if (annotator.scoped_name_resolver.scopes.back().ast_node->type ==
        bonk::TreeNodeType::n_hive_definition)
        annotator.handle_definition(node);
}