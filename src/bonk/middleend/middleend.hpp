#pragma once

namespace bonk {

class MiddleEnd;

}

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
};

struct SymbolTable {
    std::vector<std::unique_ptr<SymbolScope>> scopes;
    std::unordered_map<TreeNode*, SymbolScope*> symbol_scopes;
    std::unordered_map<TreeNode*, TreeNode*> symbol_definitions;
    std::unordered_map<TreeNode*, std::string> symbol_names;

    TreeNode* get_definition(TreeNode* node);
};

struct TypeTable {
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

struct HiddenSymbolStorage {
    std::unordered_set<std::string> hidden_symbols;

    std::string_view get_hidden_symbol(const std::string& symbol);
};

class MiddleEnd {
  public:
    Compiler& linked_compiler;

    TypeTable type_table;
    SymbolTable symbol_table;
    IDTable id_table{*this};
    HiddenSymbolStorage hidden_text_storage;

    explicit MiddleEnd(Compiler& linked_compiler);

    ~MiddleEnd() = default;

    bool transform_ast(TreeNode* ast);
    std::unique_ptr<IRProgram> generate_hir(TreeNode* ast);

    int get_hive_field_offset(TreeNodeHiveDefinition* hive_definition, int field_index);
};

} // namespace bonk