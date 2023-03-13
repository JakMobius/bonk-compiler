
#include "json_dump_ast_visitor.hpp"

void bonk::JsonDumpAstVisitor::visit(bonk::TreeNodeProgram* node) {
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

void bonk::JsonDumpAstVisitor::visit(bonk::TreeNodeHelp* node) {
    dump_node_location(node);
    dump_type(node, "Help");

    serializer.field("identifier").block_string_field() << node->identifier;
}

void bonk::JsonDumpAstVisitor::visit(bonk::TreeNodeIdentifier* node) {
    dump_node_location(node);
    dump_type(node, "Identifier");

    serializer.field("identifier_text").block_string_field() << node->identifier_text;
}

void bonk::JsonDumpAstVisitor::visit(bonk::TreeNodeBlockDefinition* node) {
    dump_node_location(node);
    dump_type(node, "BlockDefinition");

    accept_node_or_null("block_name", node->block_name);
    accept_node_or_null("block_parameters", node->block_parameters);
    accept_node_or_null("body", node->body);
}

void bonk::JsonDumpAstVisitor::visit(bonk::TreeNodeVariableDefinition* node) {
    dump_node_location(node);
    dump_type(node, "VariableDefinition");

    accept_node_or_null("variable_name", node->variable_name);
    accept_node_or_null("variable_value", node->variable_value);
    accept_node_or_null("variable_type", node->variable_type);
}

void bonk::JsonDumpAstVisitor::visit(bonk::TreeNodeParameterListDefinition* node) {
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

void bonk::JsonDumpAstVisitor::visit(bonk::TreeNodeParameterList* node) {
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
    serializer.close_array();
}

void bonk::JsonDumpAstVisitor::visit(bonk::TreeNodeParameterListItem* node) {
    dump_node_location(node);
    dump_type(node, "ParameterListItem");

    serializer.field("parameter_name").block_string_field() << node->parameter_name;
    accept_node_or_null("parameter_value", node->parameter_value);
}

void bonk::JsonDumpAstVisitor::visit(bonk::TreeNodeCodeBlock* node) {
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

void bonk::JsonDumpAstVisitor::visit(bonk::TreeNodeArrayConstant* node) {
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

void bonk::JsonDumpAstVisitor::visit(bonk::TreeNodeNumberConstant* node) {
    dump_node_location(node);
    dump_type(node, "NumberConstant");

    serializer.field("double_value").block_string_field() << node->double_value;
    serializer.field("integer_value").block_string_field() << node->integer_value;
}

void bonk::JsonDumpAstVisitor::visit(bonk::TreeNodeStringConstant* node) {
    dump_node_location(node);
    dump_type(node, "StringConstant");

    serializer.field("string_value").block_string_field() << node->string_value;
}

void bonk::JsonDumpAstVisitor::visit(bonk::TreeNodeBinaryOperation* node) {
    dump_node_location(node);
    dump_type(node, "BinaryOperation");

    serializer.field("operator_type").block_string_field()
        << BONK_OPERATOR_NAMES[(int)node->operator_type];
    accept_node_or_null("left", node->left);
    accept_node_or_null("right", node->right);
}

void bonk::JsonDumpAstVisitor::visit(bonk::TreeNodeUnaryOperation* node) {
    dump_node_location(node);
    dump_type(node, "UnaryOperation");

    serializer.field("operator_type").block_string_field()
        << BONK_OPERATOR_NAMES[(int)node->operator_type];
    accept_node_or_null("operand", node->operand);
}

void bonk::JsonDumpAstVisitor::visit(bonk::TreeNodePrimitiveType* node) {
    dump_node_location(node);
    dump_type(node, "PrimitiveType");

    serializer.field("primitive_type").block_string_field()
        << BONK_PRIMITIVE_TYPE_NAMES[(int)node->primitive_type];
}

void bonk::JsonDumpAstVisitor::visit(bonk::TreeNodeManyType* node) {
    dump_node_location(node);
    dump_type(node, "ManyType");

    accept_node_or_null("parameter", node->parameter);
}

void bonk::JsonDumpAstVisitor::visit(bonk::TreeNodeHiveAccess* node) {
    dump_node_location(node);
    dump_type(node, "HiveAccess");

    accept_node_or_null("hive", node->hive);
    accept_node_or_null("field", node->field);
}

void bonk::JsonDumpAstVisitor::visit(bonk::TreeNodeBonkStatement* node) {
    dump_node_location(node);
    dump_type(node, "BonkStatement");

    accept_node_or_null("expression", node->expression);
}

void bonk::JsonDumpAstVisitor::visit(bonk::TreeNodeLoopStatement* node) {
    dump_node_location(node);
    dump_type(node, "LoopStatement");

    accept_node_or_null("loop_parameters", node->loop_parameters);
    accept_node_or_null("loop_body", node->body);
}

void bonk::JsonDumpAstVisitor::visit(bonk::TreeNodeHiveDefinition* node) {
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

void bonk::JsonDumpAstVisitor::visit(bonk::TreeNodeCall* node) {
    dump_node_location(node);
    dump_type(node, "Call");

    accept_node_or_null("callee", node->callee);
    accept_node_or_null("arguments", node->arguments);
}

void bonk::JsonDumpAstVisitor::dump_type(bonk::TreeNode* node, std::string_view type) {
    serializer.field("type").block_string_field() << type;
}

void bonk::JsonDumpAstVisitor::dump_node_location(bonk::TreeNode* node) {
    serializer.field("source_position").block_string_field() << node->source_position;
}
