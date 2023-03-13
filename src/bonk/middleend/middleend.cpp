
#include "middleend.hpp"
#include "bonk/middleend/annotators/basic_symbol_annotator.hpp"
#include "bonk/middleend/annotators/type_annotator.hpp"
#include "bonk/middleend/converters/hive_constructor_call_replacer.hpp"
#include "bonk/middleend/converters/hive_constructor_generator.hpp"
#include "bonk/middleend/converters/stdlib_header_generator.hpp"
#include "bonk/middleend/ir/hir_generator_visitor.hpp"

bonk::MiddleEnd::MiddleEnd(bonk::Compiler& linked_compiler) : linked_compiler(linked_compiler) {
}

bool bonk::MiddleEnd::transform_ast(TreeNode* ast) {

    bonk::StdLibHeaderGenerator(*this).generate(ast);
    if (linked_compiler.state) return false;

    bonk::HiveConstructorGenerator(*this).generate(ast);
    if (linked_compiler.state) return false;

    bonk::BasicSymbolAnnotator(*this).annotate_program(ast);
    if (linked_compiler.state) return false;

    bonk::HiveConstructorCallReplacer(*this).replace(ast);
    if (linked_compiler.state) return false;

    bonk::TypeAnnotator(*this).annotate_ast(ast);
    if (linked_compiler.state) return false;

    return true;
}

std::unique_ptr<bonk::IRProgram> bonk::MiddleEnd::generate_hir(TreeNode* ast) {
    return bonk::HIRGeneratorVisitor(*this).generate(ast);
}

long long bonk::IDTable::get_unused_id() {
    return ids_used++;
}

long long bonk::IDTable::get_id(bonk::TreeNode* node) {
    auto def = middle_end.symbol_table.get_definition(node);

    if (def) {
        node = def;
    }

    auto it = ids.find(node);
    if (it != ids.end())
        return it->second;

    long long id = get_unused_id();
    ids.insert({node, id});
    nodes.insert({id, node});
    return id;
}

bonk::TreeNode* bonk::IDTable::get_node(long long id) {
    auto it = nodes.find(id);
    if (it == nodes.end())
        return nullptr;
    return it->second;
}

bonk::TreeNode* bonk::SymbolTable::get_definition(TreeNode* node) {
    auto it = symbol_definitions.find(node);
    if (it == symbol_definitions.end())
        return nullptr;
    return it->second;
}

void bonk::TypeTable::write_to_cache(TreeNode* node, Type* type) {
    type_cache.insert({node, type});
}

void bonk::TypeTable::save_type(std::unique_ptr<Type> type) {
    type_storage.insert(std::move(type));
}

bonk::Type* bonk::TypeTable::get_type(bonk::TreeNode* node) {
    auto it = type_cache.find(node);
    if (it == type_cache.end())
        return nullptr;
    return it->second;
}

std::string_view bonk::HiddenSymbolStorage::get_hidden_symbol(const std::string& symbol) {
    // Find the symbol in the set, if it's not there, insert it
    auto it = hidden_symbols.find(symbol);
    if (it == hidden_symbols.end()) {
        it = hidden_symbols.insert(symbol).first;
        return *it;
    }
    return *it;
}
