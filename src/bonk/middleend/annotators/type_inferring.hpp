#pragma once

#include "basic_symbol_annotator.hpp"
#include "bonk/tree/ast_visitor.hpp"
#include "type_annotator.hpp"

namespace bonk {

class HiveFieldNameResolver : public NameResolver {
    TreeNodeHiveDefinition* hive_definition = nullptr;

  public:
    explicit HiveFieldNameResolver(TreeNodeHiveDefinition* hive_definition);
    TreeNode* get_name_definition(std::string_view name) override;
};

class FunctionParameterNameResolver : public NameResolver {
    BlokType* called_function = nullptr;

  public:
    explicit FunctionParameterNameResolver(BlokType* called_function);
    TreeNode* get_name_definition(std::string_view name) override;
};

/* This class performs type-checking and annotates the AST tree with types.
 * It requires `symbol_table` to be filled in by basic_symbol_annotator.
 * In turn, it populates the `symbol_table` with hive field symbols and
 * function arguments, as it's impossible to recognize them without
 * type information. */

class TypeInferringVisitor : public ASTVisitor {
    MiddleEnd& middle_end;
    std::vector<Type*> type_stack;

    // This vector is used to resolve recursive function calls.
    // Basically, type-checker assumes that blocks listed here
    // never return.
    std::vector<TreeNodeBlockDefinition*> block_stack;

    // To resolve recursive function calls, it's mandatory
    // to have a stack of type tables. They're used to
    // resolve types in assumptions, for example:
    // 'Assume function A never returns, what type should
    // the '@B and @A expression have? The answer is -
    // the return type of function B.
    std::vector<std::unique_ptr<TypeTable>> type_table_stack;

  public:
    TypeInferringVisitor(MiddleEnd& middle_end) : middle_end(middle_end) {
    }

    Type* infer_type(TreeNode* node);

    void visit(TreeNodeProgram* node) override;
    void visit(TreeNodeHelp* node) override;
    void visit(TreeNodeIdentifier* node) override;
    void visit(TreeNodeBlockDefinition* node) override;
    void visit(TreeNodeVariableDefinition* node) override;
    void visit(TreeNodeParameterListDefinition* node) override;
    void visit(TreeNodeParameterList* node) override;
    void visit(TreeNodeParameterListItem* node) override;
    void visit(TreeNodeCodeBlock* node) override;
    void visit(TreeNodeArrayConstant* node) override;
    void visit(TreeNodeNumberConstant* node) override;
    void visit(TreeNodeStringConstant* node) override;
    void visit(TreeNodeBinaryOperation* node) override;
    void visit(TreeNodeUnaryOperation* node) override;
    void visit(TreeNodePrimitiveType* node) override;
    void visit(TreeNodeManyType* node) override;
    void visit(TreeNodeHiveAccess* node) override;
    void visit(TreeNodeBonkStatement* node) override;
    void visit(TreeNodeLoopStatement* node) override;
    void visit(TreeNodeHiveDefinition* node) override;
    void visit(TreeNodeCall* node) override;

    TypeTable& get_current_type_table();
    void push_type_table();
    void pop_type_table();
    std::unique_ptr<bonk::Type> infer_block_return_type(TreeNodeBlockDefinition* node);
};
} // namespace bonk