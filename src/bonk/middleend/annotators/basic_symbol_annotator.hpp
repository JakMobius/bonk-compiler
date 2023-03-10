#pragma once

namespace bonk {

class MiddleEnd;

}

#include <unordered_map>
#include "bonk/tree/ast_visitor.hpp"

namespace bonk {

class NameResolver {
  public:
    virtual TreeNode* get_name_definition(std::string_view name) = 0;
};

class Scope {
  public:
    std::unordered_map<std::string_view, TreeNode*> variables;
    TreeNode* ast_node;

    Scope(TreeNode* ast_node) : ast_node(ast_node) {}
};

class ScopedNameResolver : public NameResolver {
  public:
    std::vector<Scope> scopes;

    ScopedNameResolver();

    void push_scope(TreeNode* ast_node);
    void pop_scope();
    TreeNode* get_name_definition(std::string_view name) override;
    void define_variable(std::string_view name, TreeNode* definition);
};

class BasicSymbolAnnotator : ASTVisitor {

  public:
    ScopedNameResolver scoped_name_resolver;
    MiddleEnd& middleend;

    BasicSymbolAnnotator(MiddleEnd& middleend);

    void annotate_ast(TreeNode* ast);

    void visit(bonk::TreeNodeProgram* node) override;
    void visit(bonk::TreeNodeCodeBlock* node) override;
    void visit(bonk::TreeNodeHiveDefinition* node) override;
    void visit(bonk::TreeNodeVariableDefinition* node) override;
    void visit(bonk::TreeNodeBlockDefinition* node) override;
    void visit(bonk::TreeNodeLoopStatement* node) override;
    void visit(bonk::TreeNodeParameterListItem* node) override;
    void visit(bonk::TreeNodeIdentifier* node) override;
    void visit(bonk::TreeNodeHiveAccess* node) override;

    std::string_view get_definition_identifier(TreeNode* definition);

    void handle_definition(TreeNode* node);
    std::string name_for_def_in_current_scope(TreeNode* node);
};

} // namespace bonk

class ForwardDeclaringSymbolAnnotator : public bonk::ASTVisitor {

    bonk::BasicSymbolAnnotator& annotator;
    bool should_visit = true;

  public:

    ForwardDeclaringSymbolAnnotator(bonk::BasicSymbolAnnotator& annotator) : annotator(annotator){};

    bool visit_guard();
    void visit(bonk::TreeNodeProgram* node) override;
    void visit(bonk::TreeNodeHiveDefinition* node) override;
    void visit(bonk::TreeNodeBlockDefinition* node) override;
    void visit(bonk::TreeNodeVariableDefinition* node) override;
};