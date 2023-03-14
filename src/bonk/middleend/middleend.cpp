
#include "middleend.hpp"
#include "bonk/middleend/annotators/basic_symbol_annotator.hpp"
#include "bonk/middleend/annotators/type_annotator.hpp"
#include "bonk/middleend/annotators/type_visitor.hpp"
#include "bonk/middleend/converters/hive_constructor_call_replacer.hpp"
#include "bonk/middleend/converters/hive_ctor_dtor_early_generator.hpp"
#include "bonk/middleend/converters/hive_ctor_dtor_late_generator.hpp"
#include "bonk/middleend/converters/stdlib_header_generator.hpp"
#include "bonk/middleend/ir/hir_early_generator_visitor.hpp"
#include "bonk/middleend/ir/hir_ref_count_replacer.hpp"

bonk::MiddleEnd::MiddleEnd(bonk::Compiler& linked_compiler) : linked_compiler(linked_compiler) {
}

bool bonk::MiddleEnd::transform_ast(TreeNode* ast) {

    bonk::StdLibHeaderGenerator(*this).generate(ast);

    if (linked_compiler.state)
        return false;
    if (linked_compiler.config.should_stop_after("midend-ast-stdlib-gen"))
        return true;

    bonk::HiveConstructorDestructorEarlyGenerator(*this).generate(ast);

    if (linked_compiler.state)
        return false;
    if (linked_compiler.config.should_stop_after("midend-ast-ctor-dtor-early"))
        return true;

    bonk::BasicSymbolAnnotator(*this).annotate_program(ast);

    if (linked_compiler.state)
        return false;
    if (linked_compiler.config.should_stop_after("midend-ast-symbol-annotate"))
        return true;

    bonk::HiveConstructorCallReplacer(*this).replace(ast);

    if (linked_compiler.state)
        return false;
    if (linked_compiler.config.should_stop_after("midend-ast-ctor-call-replace"))
        return true;

    bonk::TypeAnnotator(*this).annotate_ast(ast);

    if (linked_compiler.state)
        return false;
    if (linked_compiler.config.should_stop_after("midend-ast-type-annotate"))
        return true;

    bonk::HiveConstructorDestructorLateGenerator(*this).generate(ast);

    if (linked_compiler.state)
        return false;
    if (linked_compiler.config.should_stop_after("midend-ast-ctor-dtor-late"))
        return true;

    return true;
}

std::unique_ptr<bonk::IRProgram> bonk::MiddleEnd::generate_hir(TreeNode* ast) {
    std::unique_ptr<bonk::IRProgram> program;

    program = bonk::HIREarlyGeneratorVisitor(*this).generate(ast);

    if (linked_compiler.config.should_stop_after("midend-hir-early-gen"))
        return program;

    HIRRefCountReplacer(*this).replace_ref_counters(*program);

    if (linked_compiler.config.should_stop_after("midend-hir-ref-count-replace"))
        return program;

    return program;
}

int bonk::MiddleEnd::get_hive_field_offset(bonk::TreeNodeHiveDefinition* hive_definition,
                                           int field_index) {
    int offset = 0;

    for (auto& it : hive_definition->body) {
        if (it->type != TreeNodeType::n_variable_definition)
            continue;
        if (field_index == 0)
            break;
        field_index--;

        auto variable_definition = (TreeNodeVariableDefinition*)it.get();
        auto variable_type = type_table.get_type(variable_definition);

        int footprint = FootprintCounter().get_footprint(variable_type);

        offset += footprint;

        // Align the field to its size
        offset = (offset + footprint - 1) / footprint * footprint;
    }

    return offset;
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
    if (it == type_cache.end()) {
        if (parent_table) {
            return parent_table->get_type(node);
        }
        return nullptr;
    }
    return it->second;
}
void bonk::TypeTable::sink_types_to_parent_table() {
    // Moves all types from current table to the parent table, apart from 'never'
    // types. This allows to reduce complexity of type checking.

    if (!parent_table)
        return;

    for (auto it = type_storage.begin(); it != type_storage.end();) {
        if (NeverSearchVisitor().search(it->get())) {
            ++it;
            continue;
        }

        auto node_handle = type_storage.extract(it++);
        parent_table->type_storage.insert(std::move(node_handle.value()));
    }

    for (auto it = type_cache.begin(); it != type_cache.end();) {
        if (NeverSearchVisitor().search(it->second)) {
            ++it;
            continue;
        }

        parent_table->type_cache.insert({it->first, it->second});
        it = type_cache.erase(it);
    }
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
