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

    void dump_node_location(TreeNode* node) {
        serializer.field("source_position").block_string_field() << node->source_position;
    }

    void visit(TreeNodeList* node) override {
        dump_node_location(node);
        serializer.field("type").block_string_field() << "list";
        serializer.field("contents").block_start_array();

        for (auto& element : node->list) {
            if (element) {
                serializer.array_add_block();
                element->accept(this);
                serializer.close_block();
            } else
                serializer.array_add_null();
        }

        serializer.close_array();
    }

    void visit(TreeNodeBlockDefinition* node) override {
        dump_node_location(node);
        serializer.field("type").block_string_field() << "block_definition";
        if (node->block_name) {
            serializer.field("block_name").block_string_field() << node->block_name->variable_name;
        } else {
            serializer.field("block_name").block_add_null();
        }

        accept_node_or_null("body", node->body);
    }

    void visit(TreeNodeCall* node) override {
        dump_node_location(node);
        serializer.field("type").block_string_field() << "call";

        accept_node_or_null("call_function", node->call_function);
        accept_node_or_null("call_parameters", node->call_parameters);
    }

    void visit(TreeNodeCallParameter* node) override {
        dump_node_location(node);
        serializer.field("type").block_string_field() << "call_parameter";

        accept_node_or_null("parameter_name", node->parameter_name);
        accept_node_or_null("parameter_value", node->parameter_value);
    }

    void visit(TreeNodeCheck* node) override {
        dump_node_location(node);
        serializer.field("type").block_string_field() << "check";

        accept_node_or_null("check_body", node->check_body);
        accept_node_or_null("or_body", node->or_body);
        accept_node_or_null("condition", node->condition);
    }

    void visit(TreeNodeCycle* node) override {
        dump_node_location(node);
        serializer.field("type").block_string_field() << "cycle";
        accept_node_or_null("body", node->body);
    }

    void visit(TreeNodeNumber* node) override {
        dump_node_location(node);

        serializer.field("type").block_string_field() << "root_list";
        serializer.field("float_value").block_number_field(node->float_value);
        serializer.field("integer_value").block_number_field(node->integer_value);
    }

    void visit(TreeNodeVariableDefinition* node) override {
        dump_node_location(node);

        serializer.field("type").block_string_field() << "var_definition";
        accept_node_or_null("variable_name", node->variable_name);
        accept_node_or_null("variable_value", node->variable_value);
    }

    void visit(TreeNodeOperator* node) override {
        dump_node_location(node);

        serializer.field("type").block_string_field() << "operator";
        serializer.field("operator_type").block_string_field() << BONK_OPERATOR_NAMES[(int)node->oper_type];

        accept_node_or_null("left", node->left);
        accept_node_or_null("right", node->right);
    }

    void visit(TreeNodeIdentifier* node) override {
        dump_node_location(node);

        std::string identifier{node->variable_name};

        serializer.field("type").block_string_field() << "identifier";
        serializer.field("identifier").block_string_field() << identifier;
    }

  private:
    template<typename T>
    void accept_node_or_null(std::string_view field_name, T& node) {
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