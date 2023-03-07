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

    void visit(TreeNodeProgram* node) override {
        dump_node_location(node);
        dump_type(node, "Program");
        serializer.field("body").block_start_array();

        for (auto& element : node->body) {
            if (element) {
                serializer.array_add_block();
                element->accept(this);
                serializer.close_block();
            } else
                serializer.array_add_null();
        }

        serializer.close_array();
    }

    void visit(TreeNodeHelp* node) override {
        dump_node_location(node);
        dump_type(node, "Help");

        serializer.field("identifier").block_string_field() << node->identifier;
    }

    void visit(TreeNodeIdentifier* node) override {
        dump_node_location(node);
        dump_type(node, "Identifier");

        serializer.field("identifier_text").block_string_field() << node->identifier_text;
    }

    void visit(TreeNodeBlockDefinition* node) override {
        dump_node_location(node);
        dump_type(node, "BlockDefinition");

        serializer.field("block_name").block_string_field() << node->block_name;
        accept_node_or_null("block_parameters", node->block_parameters);
        accept_node_or_null("body", node->body);
    }

    void visit(TreeNodeVariableDefinition* node) override {
        dump_node_location(node);
        dump_type(node, "VariableDefinition");

        accept_node_or_null("variable_name", node->variable_name);
        accept_node_or_null("variable_value", node->variable_value);
        accept_node_or_null("variable_type", node->variable_type);
    }

    void visit(TreeNodeParameterListDefinition* node) override {
        dump_node_location(node);
        dump_type(node, "ParameterListDefinition");

        serializer.field("parameters").block_start_array();
        for (auto& parameter : node->parameters) {
            if (parameter) {
                serializer.array_add_block();
                parameter->accept(this);
                serializer.close_block();
            } else {
                serializer.array_add_null();
            }
        }
        serializer.close_array();
    }

    void visit(TreeNodeParameterList* node) override {
        dump_node_location(node);
        dump_type(node, "ParameterList");

        serializer.field("parameters").block_start_array();
        for (auto& parameter : node->parameters) {
            if (parameter) {
                serializer.array_add_block();
                parameter->accept(this);
                serializer.close_block();
            } else {
                serializer.array_add_null();
            }
        }
    }

    void visit(TreeNodeParameterListItem* node) override {
        dump_node_location(node);
        dump_type(node, "ParameterListItem");

        serializer.field("parameter_name").block_string_field() << node->parameter_name;
        accept_node_or_null("parameter_value", node->parameter_value);
    }

    void visit(TreeNodeCodeBlock* node) override {
        dump_node_location(node);
        dump_type(node, "CodeBlock");

        serializer.field("body").block_start_array();
        for (auto& element : node->body) {
            if (element) {
                serializer.array_add_block();
                element->accept(this);
                serializer.close_block();
            } else
                serializer.array_add_null();
        }
        serializer.close_array();
    }

    void visit(TreeNodeArrayConstant* node) override {
        dump_node_location(node);
        dump_type(node, "ArrayConstant");

        serializer.field("elements").block_start_array();
        for (auto& element : node->elements) {
            if (element) {
                serializer.array_add_block();
                element->accept(this);
                serializer.close_block();
            } else
                serializer.array_add_null();
        }
        serializer.close_array();
    }

    void visit(TreeNodeNumberConstant* node) override {
        dump_node_location(node);
        dump_type(node, "NumberConstant");

        serializer.field("double_value").block_string_field() << node->double_value;
        serializer.field("integer_value").block_string_field() << node->integer_value;
    }

    void visit(TreeNodeStringConstant* node) override {
        dump_node_location(node);
        dump_type(node, "StringConstant");

        serializer.field("string_value").block_string_field() << node->string_value;
    }

    void visit(TreeNodeBinaryOperation* node) override {
        dump_node_location(node);
        dump_type(node, "BinaryOperation");

        serializer.field("operator_type").block_string_field()
            << BONK_OPERATOR_NAMES[(int)node->operator_type];
        accept_node_or_null("left", node->left);
        accept_node_or_null("right", node->right);
    }

    void visit(TreeNodeUnaryOperation* node) override {
        dump_node_location(node);
        dump_type(node, "UnaryOperation");

        serializer.field("operator_type").block_string_field()
            << BONK_OPERATOR_NAMES[(int)node->operator_type];
        accept_node_or_null("operand", node->operand);
    }

    void visit(TreeNodePrimitiveType* node) override {
        dump_node_location(node);
        dump_type(node, "PrimitiveType");

        serializer.field("primitive_type").block_string_field()
            << BONK_PRIMITIVE_TYPE_NAMES[(int)node->primitive_type];
    }

    void visit(TreeNodeManyType* node) override {
        dump_node_location(node);
        dump_type(node, "ManyType");

        accept_node_or_null("parameter", node->parameter);
    }

    void visit(TreeNodeHiveAccess* node) override {
        dump_node_location(node);
        dump_type(node, "HiveAccess");

        accept_node_or_null("hive", node->hive);
        accept_node_or_null("field", node->field);
    }

    void visit(TreeNodeBonkStatement* node) override {
        dump_node_location(node);
        dump_type(node, "BonkStatement");

        accept_node_or_null("expression", node->expression);
    }

    void visit(TreeNodeLoopStatement* node) override {
        dump_node_location(node);
        dump_type(node, "LoopStatement");

        accept_node_or_null("loop_parameters", node->loop_parameters);
        accept_node_or_null("loop_body", node->body);
    }

    void visit(TreeNodeHiveDefinition* node) override {
        dump_node_location(node);
        dump_type(node, "HiveDefinition");

        accept_node_or_null("hive_name", node->hive_name);

        serializer.field("body").block_start_array();
        for (auto& field : node->body) {
            if (field) {
                serializer.array_add_block();
                field->accept(this);
                serializer.close_block();
            } else
                serializer.array_add_null();
        }
        serializer.close_array();

    }

    void visit(TreeNodeCall* node) override {
        dump_node_location(node);
        dump_type(node, "Call");

        accept_node_or_null("callee", node->callee);
        accept_node_or_null("arguments", node->arguments);
    }

  private:
    void dump_type(TreeNode* node, std::string_view type) {
        serializer.field("type").block_string_field() << type;
    }

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