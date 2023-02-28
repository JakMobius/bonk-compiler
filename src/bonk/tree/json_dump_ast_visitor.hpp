#pragma once

#include "ast_visitor.hpp"

namespace bonk {

class JsonDumpAstVisitor : public ASTVisitor {

    JsonSerializer* serializer;

  public:
    explicit JsonDumpAstVisitor(JsonSerializer* serializer) : serializer(serializer) {
    }

    void dump_node_location(TreeNode* node) {
        if (node->source_position) {
            const char* position_string = node->source_position->to_string();
            serializer->block_string_field("source_position", position_string);
            free((void*)position_string);
        }
    }

    void visit(TreeNodeList* node) override {
        dump_node_location(node);
        serializer->block_string_field("type", "list");
        serializer->block_start_array("contents");

        for (auto element : node->list) {
            if (element) {
                serializer->array_add_block();
                element->accept(this);
                serializer->close_block();
            } else
                serializer->array_add_string(nullptr);
        }

        serializer->close_array();
    }

    void visit(TreeNodeBlockDefinition* node) override {
        dump_node_location(node);
        serializer->block_string_field("type", "block_definition");
        serializer->block_string_field(
            "block_name", node->block_name ? node->block_name->variable_name.c_str() : nullptr);
        if (node->body) {
            serializer->block_start_block("body");
            node->body->accept(this);
            serializer->close_block();
        } else {
            serializer->block_string_field("body", nullptr);
        }
    }

    void visit(TreeNodeCall* node) override {
        dump_node_location(node);
        serializer->block_string_field("type", "call");

        if (node->call_function) {
            serializer->block_start_block("call_function");
            node->call_function->accept(this);
            serializer->close_block();
        } else {
            serializer->block_string_field("call_function", nullptr);
        }

        if (node->call_parameters) {
            serializer->block_start_block("call_parameters");
            node->call_parameters->accept(this);
            serializer->close_block();
        } else {
            serializer->block_string_field("call_parameters", nullptr);
        }
    }

    void visit(TreeNodeCallParameter* node) override {
        dump_node_location(node);
        serializer->block_string_field("type", "call_parameter");
        if (node->parameter_name) {
            serializer->block_start_block("parameter_name");
            node->parameter_name->accept(this);
            serializer->close_block();
        } else {
            serializer->block_string_field("parameter_name", nullptr);
        }

        if (node->parameter_value) {
            serializer->block_start_block("parameter_value");
            node->parameter_value->accept(this);
            serializer->close_block();
        } else {
            serializer->block_string_field("parameter_value", nullptr);
        }
    }

    void visit(TreeNodeCheck* node) override {
        dump_node_location(node);
        serializer->block_string_field("type", "check");

        if (node->check_body) {
            serializer->block_start_block("check_body");
            node->check_body->accept(this);
            serializer->close_block();
        } else {
            serializer->block_string_field("parameter_value", nullptr);
        }

        if (node->or_body) {
            serializer->block_start_block("check_body");
            node->or_body->accept(this);
            serializer->close_block();
        } else {
            serializer->block_string_field("or_body", nullptr);
        }

        if (node->condition) {
            serializer->block_start_block("condition");
            node->condition->accept(this);
            serializer->close_block();
        } else {
            serializer->block_string_field("condition", nullptr);
        }
    }

    void visit(TreeNodeCycle* node) override {
        dump_node_location(node);
        serializer->block_string_field("type", "cycle");
        if (node->body) {
            serializer->block_start_block("body");
            node->body->accept(this);
            serializer->close_block();
        } else {
            serializer->block_string_field("body", nullptr);
        }
    }

    void visit(TreeNodeNumber* node) override {
        dump_node_location(node);

        serializer->block_string_field("type", "root_list");
        serializer->block_number_field("float_value", node->float_value);
        serializer->block_number_field("integer_value", node->integer_value);
    }

    void visit(TreeNodeVariableDefinition* node) override {
        dump_node_location(node);

        serializer->block_string_field("type", "var_definition");
        if (node->variable_name) {
            serializer->block_start_block("variable_name");
            node->variable_name->accept(this);
            serializer->close_block();
        } else {
            serializer->block_string_field("variable_name", nullptr);
        }

        if (node->variable_value) {
            serializer->block_start_block("variable_value");
            node->variable_value->accept(this);
            serializer->close_block();
        } else {
            serializer->block_string_field("variable_value", nullptr);
        }
    }

    void visit(TreeNodeOperator* node) override {
        dump_node_location(node);

        serializer->block_string_field("type", "operator");
        serializer->block_string_field("operator_type", OPERATOR_TYPE_NAMES[node->oper_type]);

        if (node->left) {
            serializer->block_start_block("left");
            node->left->accept(this);
            serializer->close_block();
        } else {
            serializer->block_string_field("left", nullptr);
        }
        if (node->right) {
            serializer->block_start_block("right");
            node->right->accept(this);
            serializer->close_block();
        } else {
            serializer->block_string_field("right", nullptr);
        }
    }

    void visit(TreeNodeIdentifier* node) override {
        dump_node_location(node);

        serializer->block_string_field("type", "identifier");
        serializer->block_string_field("identifier", node->variable_name.c_str());
    }
};

} // namespace bonk