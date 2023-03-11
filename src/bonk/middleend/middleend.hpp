#pragma once

#include "annotators/basic_symbol_annotator.hpp"
#include "annotators/type_annotator.hpp"
#include "bonk/compiler.hpp"
#include "bonk/middleend/ir/ir.hpp"

namespace bonk {

struct SymbolTable {
    std::unordered_map<TreeNode*, TreeNode*> symbol_definitions;
    std::unordered_map<TreeNode*, std::string> symbol_names;

    TreeNode* get_definition(TreeNode* node);
};

struct TypeTable {
    std::unordered_map<TreeNode*, Type*> type_cache{};
    std::unordered_set<std::unique_ptr<Type>> type_storage{};

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

class MiddleEnd {
  public:
    Compiler& linked_compiler;

    SymbolTable symbol_table;
    TypeTable type_table;
    IDTable id_table {*this};

    explicit MiddleEnd(Compiler& linked_compiler);

    ~MiddleEnd() = default;

    std::unique_ptr<IRProgram> run_ast(TreeNode* ast);
};

} // namespace bonk