
#include "ast_printer.hpp"

void bonk::ASTPrinter::visit(TreeNodeHelp* node) {
    stream.get_stream() << "help ";
    node->string->accept(this);
    stream.get_stream() << "\n";
}

void bonk::ASTPrinter::visit(TreeNodeIdentifier* node) {
    stream.get_stream() << node->identifier_text;
}

void bonk::ASTPrinter::visit(TreeNodeBlockDefinition* node) {
    stream.get_stream() << "blok ";
    node->block_name->accept(this);
    if (node->block_parameters) {
        node->block_parameters->accept(this);
    }
    if (node->return_type) {
        stream.get_stream() << ": ";
        node->return_type->accept(this);
        if (node->body) {
            stream.get_stream() << " ";
        }
    }
    if (node->body) {
        node->body->accept(this);
    } else {
        stream.get_stream() << ";\n";
    }
}

void bonk::ASTPrinter::visit(TreeNodeVariableDefinition* node) {
    stream.get_stream() << "bowl ";
    node->variable_name->accept(this);
    if (node->variable_type) {
        stream.get_stream() << ": ";
        node->variable_type->accept(this);
    }
    if (node->variable_value) {
        stream.get_stream() << " = ";
        node->variable_value->accept(this);
    }
}

void bonk::ASTPrinter::visit(TreeNodeParameterListDefinition* node) {
    stream.get_stream() << "[";
    bool first = true;
    for (auto& parameter : node->parameters) {
        if (!first) {
            stream.get_stream() << ", ";
        }
        first = false;
        parameter->accept(this);
    }
    stream.get_stream() << "]";
}

void bonk::ASTPrinter::visit(TreeNodeParameterList* node) {
    stream.get_stream() << "[";
    bool first = true;
    for (auto& parameter : node->parameters) {
        if (!first) {
            stream.get_stream() << ", ";
        }
        first = false;
        parameter->accept(this);
    }
    stream.get_stream() << "]";
}

void bonk::ASTPrinter::visit(TreeNodeParameterListItem* node) {
    node->parameter_name->accept(this);
    if (node->parameter_value) {
        stream.get_stream() << " = ";
        node->parameter_value->accept(this);
    }
}

void bonk::ASTPrinter::visit(TreeNodeCodeBlock* node) {
    stream.get_stream() << "{\n";
    depth++;
    for (auto& statement : node->body) {
        padding();
        statement->accept(this);

        if (statement->type == TreeNodeType::n_loop_statement ||
            statement->type == TreeNodeType::n_code_block) {
            continue;
        }

        stream.get_stream() << ";\n";
    }
    depth--;
    padding();
    stream.get_stream() << "}\n";
}

void bonk::ASTPrinter::visit(TreeNodeArrayConstant* node) {
    stream.get_stream() << "[";
    bool first = true;
    for (auto& item : node->elements) {
        if (!first) {
            stream.get_stream() << ", ";
        }
        first = false;
        item->accept(this);
    }
    stream.get_stream() << "]";
}

void bonk::ASTPrinter::visit(TreeNodeNumberConstant* node) {
    stream.get_stream() << node->contents.double_value;
}

void bonk::ASTPrinter::visit(TreeNodeStringConstant* node) {
    stream.get_stream() << "\"";

    for (auto& character : node->string_value) {
        if (character == '\n') {
            stream.get_stream() << "\\n";
        } else if (character == '\t') {
            stream.get_stream() << "\\t";
        } else if (character == '\r') {
            stream.get_stream() << "\\r";
        } else if (character == '\0') {
            stream.get_stream() << "\\0";
        } else if (character == '\\') {
            stream.get_stream() << "\\\\";
        } else if (character == '\"') {
            stream.get_stream() << "\\\"";
        } else {
            stream.get_stream() << character;
        }
    }

    stream.get_stream() << "\"";
}

void bonk::ASTPrinter::visit(TreeNodeBinaryOperation* node) {
    stream.get_stream() << "(";
    node->left->accept(this);
    stream.get_stream() << " " << BONK_OPERATOR_NAMES[(int)node->operator_type] << " ";
    node->right->accept(this);
    stream.get_stream() << ")";
}

void bonk::ASTPrinter::visit(TreeNodeUnaryOperation* node) {
    stream.get_stream() << "(" << BONK_OPERATOR_NAMES[(int)node->operator_type] << " ";
    node->operand->accept(this);
    stream.get_stream() << ")";
}

void bonk::ASTPrinter::visit(TreeNodePrimitiveType* node) {
    stream.get_stream() << BONK_TRIVIAL_TYPE_KIND_NAMES[(int)node->primitive_type];
}

void bonk::ASTPrinter::visit(TreeNodeManyType* node) {
    stream.get_stream() << "many ";
    node->parameter->accept(this);
}

void bonk::ASTPrinter::visit(TreeNodeHiveAccess* node) {
    node->field->accept(this);
    stream.get_stream() << " of ";
    node->hive->accept(this);
}

void bonk::ASTPrinter::visit(TreeNodeBonkStatement* node) {
    stream.get_stream() << "bonk";
    if (node->expression) {
        stream.get_stream() << " ";
        node->expression->accept(this);
    }
}

void bonk::ASTPrinter::visit(TreeNodeBrekStatement* node) {
    stream.get_stream() << "brek";
}

void bonk::ASTPrinter::visit(TreeNodeLoopStatement* node) {
    stream.get_stream() << "loop ";
    if (node->loop_parameters) {
        node->loop_parameters->accept(this);
    }
    node->body->accept(this);
}

void bonk::ASTPrinter::visit(TreeNodeHiveDefinition* node) {
    stream.get_stream() << "hive ";
    node->hive_name->accept(this);
    stream.get_stream() << " {\n";
    depth++;
    for (auto& child : node->body) {
        padding();
        child->accept(this);
        if (child->type == TreeNodeType::n_variable_definition) {
            stream.get_stream() << ";\n";
        } else {
            stream.get_stream() << "\n";
        }
    }
    depth--;
    padding();
    stream.get_stream() << "}\n";
}

void bonk::ASTPrinter::visit(TreeNodeCall* node) {
    stream.get_stream() << "@";
    node->callee->accept(this);
    if (node->arguments) {
        node->arguments->accept(this);
    }
}

void bonk::ASTPrinter::padding() {
    for (int i = 0; i < depth; i++) {
        stream.get_stream() << "  ";
    }
}
void bonk::ASTPrinter::visit(bonk::TreeNodeCast* node) {
    stream.get_stream() << "cast<";
    node->target_type->accept(this);
    stream.get_stream() << ">(";
    node->operand->accept(this);
    stream.get_stream() << ")";
}

void bonk::ASTPrinter::visit(bonk::TreeNodeNull* node) {
    stream.get_stream() << "null";
}
