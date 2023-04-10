
#include "hive_constructor_call_replacer.hpp"
#include "bonk/middleend/annotators/basic_symbol_annotator.hpp"

bool bonk::HiveConstructorCallReplacer::replace(bonk::AST& ast) {
    current_ast = &ast;
    ast.root->accept(this);
    current_ast = nullptr;
    return true;
}

void bonk::HiveConstructorCallReplacer::visit(bonk::TreeNodeVariableDefinition* node) {
    // Do not visit the variable type, as we only want to replace
    // hive identifiers that are used as values.

    if (node->variable_name) {
        node->variable_name->accept(this);
    }

    if (node->variable_value) {
        node->variable_value->accept(this);
    }
}

void bonk::HiveConstructorCallReplacer::visit(bonk::TreeNodeIdentifier* node) {
    // Lookup the symbol table to see if this identifier is a hive

    auto definition = middle_end.symbol_table.symbol_definitions[node];
    if (!definition.is_local()) {
        return;
    }

    auto local_definition = definition.get_local().definition;

    if(local_definition->type != TreeNodeType::n_hive_definition) {
        return;
    }

    // If it is, replace it with a hive constructor call
    std::string_view hive_name = node->identifier_text;

    std::string constructor_name{hive_name};
    constructor_name += "$$constructor";

    std::string_view symbol = current_ast->buffer.get_symbol(constructor_name);

    node->identifier_text = symbol;

    // Now we need to update the symbol table to reflect the new name
    // of the hive constructor call. Otherwise the compiler won't notice
    // the difference (or will crash)

    auto identifier_scope = middle_end.symbol_table.get_scope_for_node(node);
    ScopedNameResolver resolver;
    resolver.current_scope = identifier_scope;

    auto* constructor_definition = resolver.get_name_definition(constructor_name);
    assert(constructor_definition);

    middle_end.symbol_table.symbol_definitions[node] = SymbolDefinition::local(constructor_definition);
}

void bonk::HiveConstructorCallReplacer::visit(bonk::TreeNodeHiveDefinition* node) {
    // Do not visit the hive name, as it's unnecessary to
    // change the name of the hive itself

    for (auto& child : node->body) {
        child->accept(this);
    }
}

void bonk::HiveConstructorCallReplacer::visit(bonk::TreeNodeBlockDefinition* node) {
    // Do not visit the block return type, as we only want to replace
    // hive identifiers that are used as values.

    if (node->block_name)
        node->block_name->accept(this);
    if (node->block_parameters)
        node->block_parameters->accept(this);
    if (node->body)
        node->body->accept(this);
}
