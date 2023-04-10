#pragma once

namespace bonk {

class MiddleEnd;

}

#include <filesystem>
#include <unordered_map>
#include <unordered_set>
#include "bonk/compiler.hpp"
#include "bonk/middleend/annotators/types.hpp"
#include "bonk/middleend/ir/ir.hpp"

namespace bonk {

struct SymbolScope {
    SymbolScope* parent_scope;
    TreeNode* definition;
    std::unordered_map<std::string_view, TreeNode*> symbols;

    TreeNodeType get_type();
};

struct LocalDefinition {
    TreeNode* definition;
};

struct ExternalDefinition {
    std::string_view file;
};

struct NoDefinition {};

struct SymbolDefinition {
    std::variant<NoDefinition, LocalDefinition, ExternalDefinition> definition;

    bool is_local() {
        return std::holds_alternative<LocalDefinition>(definition);
    }

    bool is_external() {
        return std::holds_alternative<ExternalDefinition>(definition);
    }

    LocalDefinition& get_local() {
        return std::get<LocalDefinition>(definition);
    }

    ExternalDefinition& get_external() {
        return std::get<ExternalDefinition>(definition);
    }

    static SymbolDefinition local(TreeNode* definition) {
        return SymbolDefinition{LocalDefinition{definition}};
    }

    static SymbolDefinition external(std::string_view file) {
        return SymbolDefinition{ExternalDefinition{file}};
    }

    static SymbolDefinition none() {
        return SymbolDefinition{NoDefinition{}};
    }

    operator bool() {
        return !std::holds_alternative<NoDefinition>(definition);
    }
};

struct SymbolTable {
    SymbolScope* global_scope;
    std::vector<std::unique_ptr<SymbolScope>> scopes;
    std::unordered_map<TreeNode*, SymbolScope*> symbol_scopes;
    std::unordered_map<TreeNode*, SymbolDefinition> symbol_definitions;
    std::unordered_map<TreeNode*, std::string> symbol_names;

    SymbolTable();

    SymbolScope* create_scope(bonk::TreeNode* ast_node, SymbolScope* parent_scope);
    SymbolScope* get_scope_for_node(TreeNode* node);

    SymbolDefinition get_definition(TreeNode* node);
};

struct TypeTable {
    bool has_errors = false;
    TypeTable* parent_table = nullptr;
    std::unordered_map<TreeNode*, Type*> type_cache{};
    std::unordered_set<std::unique_ptr<Type>> type_storage{};

    template <typename T> T* create() {
        auto type = std::make_unique<T>();
        T* pure_type = type.get();
        save_type(std::move(type));
        return pure_type;
    }

    template <typename T> T* annotate(TreeNode* node) {
        auto type = std::make_unique<T>();
        T* pure_type = type.get();
        save_type(std::move(type));
        write_to_cache(node, pure_type);
        return pure_type;
    }

    Type* annotate(TreeNode* node, Type* type) {
        if (!type)
            return type;
        write_to_cache(node, type);
        return type;
    }

    void write_to_cache(TreeNode* node, Type* type);
    void save_type(std::unique_ptr<Type> type);

    Type* get_type(TreeNode* node);
    void sink_types_to_parent_table();
};

struct IDTable {
    MiddleEnd& middle_end;
    std::unordered_map<TreeNode*, long long> ids{};
    std::unordered_map<long long, TreeNode*> nodes{};

    int ids_used = 0;

    explicit IDTable(MiddleEnd& middle_end) : middle_end(middle_end) {
    }

    long long get_unused_id();
    long long get_id(TreeNode* node);
    TreeNode* get_node(long long id);
};

struct ExternalSymbolTable {
    // Defined to be a vector of chars instead of std::string
    // to ensure that the data is not moved around in memory when
    // vector is resized
    std::vector<std::vector<char>> external_files;
    std::unordered_map<std::string_view, unsigned long> external_file_map;
    std::unordered_map<TreeNodeIdentifier*, unsigned long> external_symbol_def_files;

    std::string_view get_external_file(int index);
    void register_symbol(TreeNodeIdentifier* node, std::string_view filename);
};

struct ExternalModule {
    SymbolScope* scope;
    bonk::AST module_ast;

    ExternalModule(SymbolScope* scope, bonk::AST&& module)
        : scope(scope), module_ast(std::move(module)) {
    }
};

class MiddleEnd {
  public:
    Compiler& compiler;

    std::unordered_map<std::string, std::unique_ptr<ExternalModule>> external_modules;
    std::optional<std::filesystem::path> module_path = std::nullopt;

    TypeTable type_table;
    SymbolTable symbol_table;
    IDTable id_table{*this};
    ExternalSymbolTable external_symbol_table;

    explicit MiddleEnd(Compiler& linked_compiler);

    ~MiddleEnd() = default;

    // Used to transform ASTs, which are not yet transformed
    // i.e the raw AST parsed from source files
    bool transform_ast(bonk::AST& ast);

    std::unique_ptr<IRProgram> generate_hir(TreeNode* ast);

    int get_hive_field_offset(TreeNodeHiveDefinition* hive_definition, int field_index);

    bool has_module(const std::string& name);
    bool annotate_ast(AST& ast, SymbolScope* scope);
    ExternalModule* add_external_module(const std::filesystem::path& path,
                             bonk::AST module, bool disclose = true);
    ExternalModule* get_external_module(const std::filesystem::path& path);
};

} // namespace bonk