

#pragma once

namespace bonk {

struct Parser;
struct Compiler;

} // namespace bonk

#include <cassert>
#include <cstdlib>
#include <optional>
#include <vector>
#include "bonk/compiler/compiler_message_stream_proxy.hpp"
#include "bonk/frontend/parsing/lexic/lexer.hpp"
#include "bonk/frontend/ast/ast.hpp"

namespace bonk {

struct Parser {
    unsigned long position = 0;
    std::vector<Lexeme>* input = nullptr;
    Compiler& compiler;

    bool errors_occurred = false;

    Lexeme* next_lexeme();

    void eat_lexeme();

    void spit_lexeme();

    Parser(Compiler& compiler);

    std::unique_ptr<TreeNodeProgram> parse_file(std::vector<Lexeme>* lexemes);

  private:
    std::unique_ptr<TreeNodeProgram> parse_program();
    std::unique_ptr<TreeNodeHelp> parse_help_statement();
    std::unique_ptr<TreeNode> parse_definition();
    std::unique_ptr<TreeNodeBlockDefinition> parse_blok_definition();
    std::unique_ptr<TreeNodeVariableDefinition> parse_variable_definition();
    std::unique_ptr<TreeNodeParameterListDefinition> parse_parameter_list_definition();
    std::unique_ptr<TreeNodeCodeBlock> parse_code_block();
    std::unique_ptr<TreeNode> parse_statement();
    std::unique_ptr<TreeNodeBonkStatement> parse_bonk_statement();
    std::unique_ptr<TreeNodeBrekStatement> parse_brek_statement();
    std::unique_ptr<TreeNodeArrayConstant> parse_array_constant();
    std::unique_ptr<TreeNodeParameterList> parse_parameter_list();
    std::unique_ptr<TreeNodeParameterListItem> parse_parameter_list_item();
    std::unique_ptr<TreeNodeLoopStatement> parse_loop_statement();
    std::unique_ptr<TreeNode> parse_type();
    std::unique_ptr<TreeNodeHiveDefinition> parse_hive_definition();

    std::unique_ptr<TreeNode> parse_expression();
    std::unique_ptr<TreeNode> parse_expression_assignment();
    std::unique_ptr<TreeNode> parse_expression_or();
    std::unique_ptr<TreeNode> parse_expression_and();
    std::unique_ptr<TreeNode> parse_expression_equality();
    std::unique_ptr<TreeNode> parse_expression_relational();
    std::unique_ptr<TreeNode> parse_expression_add();
    std::unique_ptr<TreeNode> parse_expression_multiply();

    std::unique_ptr<TreeNode> parse_expression_unary();
    std::unique_ptr<TreeNode> parse_expression_primary();
    std::unique_ptr<TreeNode> parse_expression_call();

    CompilerMessageStreamProxy warning() const;
    CompilerMessageStreamProxy error();
    CompilerMessageStreamProxy fatal_error();

    template <typename NextExpression>
    std::unique_ptr<TreeNode>
    parse_operator_expression(const NextExpression& next_expression_parser,
                              const std::initializer_list<OperatorType>& operator_filter,
                              bool right_associative = false) {
        std::unique_ptr<TreeNode> result = next_expression_parser();

        if(!result) {
            return nullptr;
        }

        bool first_operation = true;

        while(true) {
            bool found_operator = false;

            for (OperatorType operator_type : operator_filter) {
                if (next_lexeme()->is(operator_type)) {
                    found_operator = true;
                    break;
                }
            }

            if (!found_operator) {
                return result;
            }

            OperatorType operator_type = std::get<OperatorLexeme>(next_lexeme()->data).type;
            ParserPosition source_position = next_lexeme()->start_position;
            eat_lexeme();

            auto next_expression = next_expression_parser();
            if (!next_expression)
                return nullptr;

            if(right_associative && !first_operation) {
                auto* last_binary_op = (TreeNodeBinaryOperation*) result.get();

                auto new_binary_op = std::make_unique<TreeNodeBinaryOperation>();
                new_binary_op->source_position = source_position;
                new_binary_op->operator_type = operator_type;
                new_binary_op->left = std::move(last_binary_op->right);
                new_binary_op->right = std::move(next_expression);
                last_binary_op->right = std::move(new_binary_op);
            } else {
                auto binary_op = std::make_unique<TreeNodeBinaryOperation>();
                binary_op->source_position = source_position;
                binary_op->operator_type = operator_type;
                binary_op->left = std::move(result);
                binary_op->right = std::move(next_expression);
                result = std::move(binary_op);
            }

            first_operation = false;
        };

        return result;
    }
};

} // namespace bonk

// TODO: Get rid of trailing includes
#include "bonk/compiler/compiler.hpp"
