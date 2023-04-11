
#include "frontend.hpp"
#include "bonk/frontend/annotators/basic_symbol_annotator.hpp"
#include "bonk/frontend/annotators/type_annotator.hpp"
#include "bonk/frontend/annotators/type_visitor.hpp"
#include "bonk/frontend/converters/external_type_replacer.hpp"
#include "bonk/frontend/converters/hir_early_generator_visitor.hpp"
#include "bonk/frontend/converters/hive_constructor_call_replacer.hpp"
#include "bonk/frontend/converters/hive_ctor_dtor_early_generator.hpp"
#include "bonk/frontend/converters/hive_ctor_dtor_late_generator.hpp"
#include "bonk/frontend/converters/stdlib_header_generator.hpp"

bonk::FrontEnd::FrontEnd(bonk::Compiler& linked_compiler) : compiler(linked_compiler) {
    add_external_module("$$stdlib", bonk::StdLibHeaderGenerator(*this).generate());
}

bool bonk::FrontEnd::transform_ast(bonk::AST& ast) {

    if (!bonk::HiveConstructorDestructorEarlyGenerator(*this).generate(ast))
        return false;

    if (!bonk::BasicSymbolAnnotator(*this).annotate_ast(ast))
        return false;

    if (!bonk::HiveConstructorCallReplacer(*this).replace(ast))
        return false;

    if (!bonk::TypeAnnotator(*this).annotate_ast(ast))
        return false;

    if (!bonk::HiveConstructorDestructorLateGenerator(*this).generate(ast))
        return false;

    if (!bonk::ExternalTypeReplacer(*this).replace(ast))
        return false;

    return true;
}

bool bonk::FrontEnd::annotate_ast(AST& ast, SymbolScope* scope) {
    bonk::BasicSymbolAnnotator symbol_annotator(*this);

    if (scope) {
        symbol_annotator.scoped_name_resolver.current_scope = scope;
    }

    if (!symbol_annotator.annotate_ast(ast))
        return false;

    return bonk::TypeAnnotator(*this).annotate_ast(ast);
}

bonk::ExternalModule* bonk::FrontEnd::add_external_module(const std::filesystem::path& path,
                                                           bonk::AST ast, bool disclose) {
    auto scope = symbol_table.global_scope;

    // If the module is imported from another module, we don't want to disclose its symbols,
    // so we create a new anonymous scope for it

    if (!disclose) {
        scope = symbol_table.create_scope(nullptr, scope);
    }

    if (!annotate_ast(ast, scope)) {
        return nullptr;
    }

    auto module = std::make_unique<ExternalModule>(scope, std::move(ast));
    auto module_ptr = module.get();

    external_modules.insert({path.string(), std::move(module)});

    return module_ptr;
}

std::unique_ptr<bonk::HIRProgram> bonk::FrontEnd::generate_hir(TreeNode* ast) {
    return bonk::HIREarlyGeneratorVisitor(*this).generate(ast);
}

int bonk::FrontEnd::get_hive_field_offset(bonk::TreeNodeHiveDefinition* hive_definition,
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

bool bonk::FrontEnd::has_module(const std::string& name) {
    return external_modules.find(name) != external_modules.end();
}

bonk::ExternalModule* bonk::FrontEnd::get_external_module(const std::filesystem::path& path) {
    auto it = external_modules.find(path.string());
    if (it == external_modules.end())
        return nullptr;
    return it->second.get();
}

long long bonk::IDTable::get_unused_id() {
    return ids_used++;
}

long long bonk::IDTable::get_id(bonk::TreeNode* node) {
    auto def = front_end.symbol_table.get_definition(node);

    if (def) {
        // Definition must be local at this point, otherwise it's a bug
        node = def.get_local().definition;
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

bonk::SymbolDefinition bonk::SymbolTable::get_definition(TreeNode* node) {
    auto it = symbol_definitions.find(node);
    if (it == symbol_definitions.end())
        return bonk::SymbolDefinition::none();
    return it->second;
}

bonk::SymbolTable::SymbolTable() {
    global_scope = create_scope(nullptr, nullptr);
}

bonk::SymbolScope* bonk::SymbolTable::create_scope(bonk::TreeNode* ast_node, SymbolScope* parent) {
    auto new_scope = std::make_unique<SymbolScope>();
    auto scope_ptr = new_scope.get();

    new_scope->definition = ast_node;
    scopes.push_back(std::move(new_scope));

    if (ast_node) {
        symbol_scopes[ast_node] = scope_ptr;
    }

    scope_ptr->parent_scope = parent;

    return scope_ptr;
}

bonk::SymbolScope* bonk::SymbolTable::get_scope_for_node(bonk::TreeNode* node) {
    auto it = symbol_scopes.find(node);
    if (it == symbol_scopes.end())
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

bonk::TreeNodeType bonk::SymbolScope::get_type() {
    if (!definition) {
        return TreeNodeType::n_program;
    } else {
        return definition->type;
    }
}

void bonk::ExternalSymbolTable::register_symbol(bonk::TreeNodeIdentifier* node,
                                                std::string_view filename) {
    auto it = external_file_map.find(filename);
    unsigned long file_index = 0;
    if (it == external_file_map.end()) {
        file_index = external_files.size();
        std::vector<char> filename_copy(filename.begin(), filename.end());
        std::string_view safe_string_view(filename_copy.data(), filename_copy.size());
        external_file_map.insert({safe_string_view, file_index});
        external_files.push_back(std::move(filename_copy));
    } else {
        file_index = it->second;
    }

    external_symbol_def_files.insert({node, file_index});
}

std::string_view bonk::ExternalSymbolTable::get_external_file(int index) {
    auto& storage = external_files[index];
    return {storage.data(), storage.size()};
}
