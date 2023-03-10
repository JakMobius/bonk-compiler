#pragma once

#include <sstream>
#include "ast.hpp"
#include "ast_visitor.hpp"
#include "utils/json_serializer.hpp"

namespace bonk {

class JsonDumpAstVisitor : public ASTVisitor {

    JsonSerializer& serializer;

  public:
    explicit JsonDumpAstVisitor(JsonSerializer& serializer) : serializer(serializer) {
    }

    void dump_node_location(TreeNode* node);

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

  private:
    void dump_type(TreeNode* node, std::string_view type);

    template <typename T> void accept_node_or_null(std::string_view field_name, T& node) {
        if (node) {
            serializer.field(field_name).block_start_block();
            node->accept(this);
            serializer.close_block();
        } else {
            serializer.field(field_name).block_add_null();
        }
    }
};

} // namespace bonk