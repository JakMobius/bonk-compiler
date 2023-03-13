#pragma once

namespace bonk {

class TypeAnnotator;
class MiddleEnd;

}

#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include "bonk/tree/ast_visitor.hpp"
#include "types.hpp"

namespace bonk {



/* This class performs type-checking and annotates the AST tree with types.
 * It requires `symbol_table` to be filled in by basic_symbol_annotator.
 * In turn, it populates the `symbol_table` with hive field symbols and
 * function arguments, as it's impossible to recognize them without
 * type information. */

class TypeAnnotator : ASTVisitor {

  public:
    MiddleEnd& middle_end;

    explicit TypeAnnotator(MiddleEnd& middle_end);
    ~TypeAnnotator() = default;

    Type* infer_type(TreeNode* node);

    void visit(TreeNodeHiveDefinition* node) override;
    void visit(TreeNodeVariableDefinition* node) override;
    void visit(TreeNodeBlockDefinition* node) override;
    void visit(TreeNodeHiveAccess* node) override;
    void visit(TreeNodeCall* node) override;
    void visit(TreeNodeParameterListItem* node) override;

    void visit(TreeNodeArrayConstant* node) override;
    void visit(TreeNodeNumberConstant* node) override;
    void visit(TreeNodeStringConstant* node) override;
    void visit(TreeNodeBinaryOperation* node) override;
    void visit(TreeNodeUnaryOperation* node) override;
    void visit(TreeNodePrimitiveType* node) override;
    void visit(TreeNodeManyType* node) override;
    void visit(TreeNodeBonkStatement* node) override;
    void annotate_ast(TreeNode* ast);
};

} // namespace bonk