

#pragma once

namespace bonk {

struct Parser;
struct Compiler;

} // namespace bonk

#include <cassert>
#include <cstdlib>
#include <optional>
#include <vector>
#include "./lexic/lexical_analyzer.hpp"

namespace bonk {

struct Parser {
    unsigned long position = 0;
    std::vector<Lexeme>* input = nullptr;
    Compiler& linked_compiler;

    Lexeme* next_lexeme();

    void eat_lexeme();

    void spit_lexeme();

    Parser(Compiler& compiler);

    std::unique_ptr<TreeNode> parse_file(std::vector<Lexeme>* lexemes);

  private:
    std::unique_ptr<TreeNode> parse_program();
    std::unique_ptr<TreeNodeHelp> parse_help_statement();
    std::unique_ptr<TreeNode> parse_definition();
    std::unique_ptr<TreeNodeBlockDefinition> parse_blok_definition();
    std::unique_ptr<TreeNodeVariableDefinition> parse_variable_definition();
    std::unique_ptr<TreeNodeParameterListDefinition> parse_parameter_list_definition();
    std::unique_ptr<TreeNodeCodeBlock> parse_code_block();
    std::unique_ptr<TreeNode> parse_statement();
    std::unique_ptr<TreeNodeBonkStatement> parse_bonk_statement();
    std::unique_ptr<TreeNodeArrayConstant> parse_array_constant();
    std::unique_ptr<TreeNodeHiveAccess> parse_hive_access();
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

    template <typename NextExpression>
    std::unique_ptr<TreeNode>
    parse_operator_expression(const NextExpression& next_expression_parser,
                              const std::initializer_list<OperatorType>& operator_filter) {
        // OperatorExpression<NextExpression, Operator>:
        //  NextExpression |
        //  NextExpression Operator OperatorExpression<NextExpression, Operator>

        auto left_hand_expression = next_expression_parser();
        if (!left_hand_expression)
            return nullptr;

        bool found_operator = false;

        for (OperatorType operator_type : operator_filter) {
            if(next_lexeme()->is(operator_type)) {
                found_operator = true;
                break;
            }
        }

        if(!found_operator) {
            return left_hand_expression;
        }

        OperatorType operator_type = std::get<OperatorLexeme>(next_lexeme()->data).type;
        eat_lexeme();

        auto right_hand_expression = parse_operator_expression<NextExpression>(
            next_expression_parser, operator_filter);
        if (!right_hand_expression)
            return nullptr;

        auto result = std::make_unique<TreeNodeBinaryOperation>();
        result->left = std::move(left_hand_expression);
        result->right = std::move(right_hand_expression);
        result->operator_type = operator_type;

        return result;
    }
};

} // namespace bonk

// TODO: Get rid of trailing includes
#include "../compiler.hpp"
