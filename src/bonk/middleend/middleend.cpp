
#include "middleend.hpp"
#include "bonk/middleend/ir/hir_generator_visitor.hpp"
#include "bonk/middleend/ir/hir_printer.hpp"

bonk::MiddleEnd::MiddleEnd(bonk::Compiler& linked_compiler) : linked_compiler(linked_compiler) {
}

bool bonk::MiddleEnd::run_ast(TreeNode* ast) {

    bonk::BasicSymbolAnnotator symbol_annotator{*this};
    bonk::TypeAnnotator type_annotator{*this};

    symbol_annotator.annotate_ast(ast);

    if (linked_compiler.state) {
        return false;
    }

    type_annotator.annotate_ast(ast);

    if (linked_compiler.state) {
        return false;
    }

    bonk::HIRGeneratorVisitor hir_generator{*this};
    IRProgram program{};
    hir_generator.generate(ast, &program);

    StdOutputStream stream{std::cout};
    HIRPrinter printer{stream};
    printer.middle_end = this;
    printer.print(program);

    return true;
}

long long bonk::IDTable::get_unused_id() {
    return ids_used++;
}

long long bonk::IDTable::get_id(bonk::TreeNode* node) {
    auto def = middle_end.symbol_table.get_definition(node) ;

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
