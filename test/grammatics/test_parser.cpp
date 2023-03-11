
#include <gtest/gtest.h>
#include "bonk/parsing/parser.hpp"
#include "bonk/tree/json_dump_ast_visitor.hpp"

TEST(Parser, TestHiveBowls) {
    auto error_stream = bonk::StdOutputStream(std::cout);

    bonk::CompilerConfig config{.error_file = error_stream};
    bonk::Compiler compiler(config);

    const char* source = R"(
        hive TestHive {
            bowl test_bowl: flot;
            bowl test_bowl2: nubr;
            bowl test_bowl3: strg;
            bowl test_bowl4: many strg;
            bowl test_bowl5 = "test";
        }
    )";

    auto lexemes = bonk::LexicalAnalyzer(compiler).parse_file("test", source);
    auto ast = bonk::Parser(compiler).parse_file(&lexemes);

    ASSERT_TRUE(compiler.state == bonk::BONK_COMPILER_OK);
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(ast->type, bonk::TreeNodeType::n_program);

    auto program = (bonk::TreeNodeProgram*)ast.get();
    ASSERT_EQ(program->body.size(), 1);
    ASSERT_EQ(program->body.front()->type, bonk::TreeNodeType::n_hive_definition);

    auto hive_definition = (bonk::TreeNodeHiveDefinition*)program->body.front().get();
    ASSERT_EQ(hive_definition->hive_name->identifier_text, "TestHive");
    ASSERT_EQ(hive_definition->body.size(), 5);

    std::vector<bonk::TreeNodeVariableDefinition*> variable_definitions{};
    bonk::TreeNodePrimitiveType* primitive_type = nullptr;
    bonk::TreeNodeManyType* many_type = nullptr;
    bonk::TreeNodeStringConstant* string_constant = nullptr;
    bonk::TreeNodeVariableDefinition* definition = nullptr;

    for (auto& child : hive_definition->body) {
        ASSERT_EQ(child->type, bonk::TreeNodeType::n_variable_definition);
        variable_definitions.push_back((bonk::TreeNodeVariableDefinition*)child.get());
    }

    definition = variable_definitions[0];
    ASSERT_EQ(definition->variable_name->identifier_text, "test_bowl");
    ASSERT_EQ(definition->variable_type->type, bonk::TreeNodeType::n_primitive_type);
    ASSERT_EQ(definition->variable_value, nullptr);
    primitive_type = (bonk::TreeNodePrimitiveType*)definition->variable_type.get();
    ASSERT_EQ(primitive_type->primitive_type, bonk::PrimitiveType::t_flot);

    definition = variable_definitions[1];
    ASSERT_EQ(definition->variable_name->identifier_text, "test_bowl2");
    ASSERT_EQ(definition->variable_type->type, bonk::TreeNodeType::n_primitive_type);
    ASSERT_EQ(definition->variable_value, nullptr);
    primitive_type = (bonk::TreeNodePrimitiveType*)definition->variable_type.get();
    ASSERT_EQ(primitive_type->primitive_type, bonk::PrimitiveType::t_nubr);

    definition = variable_definitions[2];
    ASSERT_EQ(definition->variable_name->identifier_text, "test_bowl3");
    ASSERT_EQ(definition->variable_type->type, bonk::TreeNodeType::n_primitive_type);
    ASSERT_EQ(definition->variable_value, nullptr);
    primitive_type = (bonk::TreeNodePrimitiveType*)definition->variable_type.get();
    ASSERT_EQ(primitive_type->primitive_type, bonk::PrimitiveType::t_strg);

    definition = variable_definitions[3];
    ASSERT_EQ(definition->variable_name->identifier_text, "test_bowl4");
    ASSERT_EQ(definition->variable_type->type, bonk::TreeNodeType::n_many_type);
    ASSERT_EQ(definition->variable_value, nullptr);
    many_type = (bonk::TreeNodeManyType*)definition->variable_type.get();
    ASSERT_EQ(many_type->parameter->type, bonk::TreeNodeType::n_primitive_type);
    primitive_type = (bonk::TreeNodePrimitiveType*)many_type->parameter.get();
    ASSERT_EQ(primitive_type->primitive_type, bonk::PrimitiveType::t_strg);

    definition = variable_definitions[4];
    ASSERT_EQ(definition->variable_name->identifier_text, "test_bowl5");
    ASSERT_EQ(definition->variable_type, nullptr);
    ASSERT_EQ(definition->variable_value->type, bonk::TreeNodeType::n_string_constant);
    string_constant = (bonk::TreeNodeStringConstant*)definition->variable_value.get();
    ASSERT_EQ(string_constant->string_value, "test");
}

TEST(Parser, TestHiveBloks) {
    auto error_stream = bonk::StdOutputStream(std::cout);

    bonk::CompilerConfig config{.error_file = error_stream};
    bonk::Compiler compiler(config);

    const char* source = R"(
        hive TestHive {
            bowl field: flot;
            blok set_field[bowl field: flot] {
                field of me = field;
            }
        }
    )";

    auto lexemes = bonk::LexicalAnalyzer(compiler).parse_file("test", source);
    auto ast = bonk::Parser(compiler).parse_file(&lexemes);

    ASSERT_TRUE(compiler.state == bonk::BONK_COMPILER_OK);
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(ast->type, bonk::TreeNodeType::n_program);

    auto program = (bonk::TreeNodeProgram*)ast.get();
    ASSERT_EQ(program->body.size(), 1);
    ASSERT_EQ(program->body.front()->type, bonk::TreeNodeType::n_hive_definition);

    auto hive_definition = (bonk::TreeNodeHiveDefinition*)program->body.front().get();
    ASSERT_EQ(hive_definition->hive_name->identifier_text, "TestHive");
    ASSERT_EQ(hive_definition->body.size(), 2);

    bonk::TreeNodeVariableDefinition* variable_definition = nullptr;
    bonk::TreeNodeBlockDefinition* blok_definition = nullptr;

    ASSERT_EQ(hive_definition->body.front()->type, bonk::TreeNodeType::n_variable_definition);
    ASSERT_EQ(hive_definition->body.back()->type, bonk::TreeNodeType::n_block_definition);

    variable_definition = (bonk::TreeNodeVariableDefinition*)hive_definition->body.front().get();
    blok_definition = (bonk::TreeNodeBlockDefinition*)hive_definition->body.back().get();

    ASSERT_EQ(variable_definition->variable_name->identifier_text, "field");
    ASSERT_EQ(variable_definition->variable_type->type, bonk::TreeNodeType::n_primitive_type);
    ASSERT_EQ(variable_definition->variable_value, nullptr);
    ASSERT_EQ(
        ((bonk::TreeNodePrimitiveType*)variable_definition->variable_type.get())->primitive_type,
        bonk::PrimitiveType::t_flot);

    ASSERT_EQ(blok_definition->block_name->identifier_text, "set_field");
    ASSERT_EQ(blok_definition->block_parameters->parameters.size(), 1);

    auto& parameters = blok_definition->block_parameters->parameters;
    ASSERT_EQ(parameters.front()->variable_name->identifier_text, "field");
    ASSERT_EQ(parameters.front()->variable_type->type, bonk::TreeNodeType::n_primitive_type);
    ASSERT_EQ(parameters.front()->variable_value, nullptr);
    ASSERT_EQ(
        ((bonk::TreeNodePrimitiveType*)parameters.front()->variable_type.get())->primitive_type,
        bonk::PrimitiveType::t_flot);

    ASSERT_EQ(blok_definition->body->body.size(), 1);

    auto statement = blok_definition->body->body.front().get();
    ASSERT_EQ(statement->type, bonk::TreeNodeType::n_binary_operation);

    auto binary_op = (bonk::TreeNodeBinaryOperation*)statement;
    ASSERT_EQ(binary_op->operator_type, bonk::OperatorType::o_assign);
    ASSERT_EQ(binary_op->left->type, bonk::TreeNodeType::n_hive_access);

    auto hive_access = (bonk::TreeNodeHiveAccess*)binary_op->left.get();
    ASSERT_EQ(hive_access->hive->type, bonk::TreeNodeType::n_identifier);
    ASSERT_EQ(((bonk::TreeNodeIdentifier*)hive_access->hive.get())->identifier_text, "me");
    ASSERT_EQ(hive_access->field->identifier_text, "field");

    ASSERT_EQ(binary_op->right->type, bonk::TreeNodeType::n_identifier);
    ASSERT_EQ(((bonk::TreeNodeIdentifier*)binary_op->right.get())->identifier_text, "field");
}

TEST(Parser, TestHiveCalls) {
    auto error_stream = bonk::StdOutputStream(std::cout);

    bonk::CompilerConfig config{.error_file = error_stream};
    bonk::Compiler compiler(config);

    const char* source = R"(
        hive TestHive {
            blok test_blok[bowl param: strg] {}
        }

        blok test_call {
            bowl my_hive = @TestHive;
            @test_blok of my_hive[param = "test"];
        }
    )";

    auto lexemes = bonk::LexicalAnalyzer(compiler).parse_file("test", source);
    auto ast = bonk::Parser(compiler).parse_file(&lexemes);

    ASSERT_TRUE(compiler.state == bonk::BONK_COMPILER_OK);
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(ast->type, bonk::TreeNodeType::n_program);

    auto program = (bonk::TreeNodeProgram*)ast.get();
    ASSERT_EQ(program->body.size(), 2);
    ASSERT_EQ(program->body.front()->type, bonk::TreeNodeType::n_hive_definition);
    ASSERT_EQ(program->body.back()->type, bonk::TreeNodeType::n_block_definition);

    auto hive_definition = (bonk::TreeNodeHiveDefinition*)program->body.front().get();
    auto blok_definition = (bonk::TreeNodeBlockDefinition*)program->body.back().get();

    ASSERT_EQ(hive_definition->hive_name->identifier_text, "TestHive");
    ASSERT_EQ(hive_definition->body.size(), 1);
    ASSERT_EQ(hive_definition->body.front()->type, bonk::TreeNodeType::n_block_definition);

    ASSERT_EQ(blok_definition->block_name->identifier_text, "test_call");
    ASSERT_EQ(blok_definition->block_parameters, nullptr);

    ASSERT_EQ(blok_definition->body->body.size(), 2);

    auto statement = blok_definition->body->body.front().get();
    ASSERT_EQ(statement->type, bonk::TreeNodeType::n_variable_definition);

    auto variable_definition = (bonk::TreeNodeVariableDefinition*)statement;
    ASSERT_EQ(variable_definition->variable_name->identifier_text, "my_hive");
    ASSERT_EQ(variable_definition->variable_type, nullptr);
    ASSERT_EQ(variable_definition->variable_value->type, bonk::TreeNodeType::n_call);

    auto call = (bonk::TreeNodeCall*)variable_definition->variable_value.get();
    ASSERT_EQ(call->callee->type, bonk::TreeNodeType::n_identifier);
    ASSERT_EQ(call->arguments, nullptr);
    ASSERT_EQ(((bonk::TreeNodeIdentifier*)call->callee.get())->identifier_text, "TestHive");

    statement = blok_definition->body->body.back().get();
    ASSERT_EQ(statement->type, bonk::TreeNodeType::n_call);

    call = (bonk::TreeNodeCall*)statement;
    ASSERT_EQ(call->callee->type, bonk::TreeNodeType::n_hive_access);
    ASSERT_EQ(call->arguments->parameters.size(), 1);

    auto hive_access = (bonk::TreeNodeHiveAccess*)call->callee.get();
    ASSERT_EQ(hive_access->hive->type, bonk::TreeNodeType::n_identifier);
    ASSERT_EQ(((bonk::TreeNodeIdentifier*)hive_access->hive.get())->identifier_text, "my_hive");
    ASSERT_EQ(hive_access->field->identifier_text, "test_blok");

    auto& parameters = call->arguments->parameters;
    ASSERT_EQ(parameters.front()->parameter_name->identifier_text, "param");
    ASSERT_EQ(parameters.front()->parameter_value->type, bonk::TreeNodeType::n_string_constant);
    ASSERT_EQ(
        ((bonk::TreeNodeStringConstant*)parameters.front()->parameter_value.get())->string_value,
        "test");
}

TEST(Parser, TestOperatorPrecedence) {
    auto error_stream = bonk::StdOutputStream(std::cout);

    bonk::CompilerConfig config{.error_file = error_stream};
    bonk::Compiler compiler(config);

    const char* source = R"(
            blok test {
                bowl a: flot;
                a = 1 + 2 * 3;
                a = 1 * 2 and 3;
                a = 1 and 2 or 3;
                a += 2 * 3;
                a < 5 == a > 3;
            }
        )";

    auto lexemes = bonk::LexicalAnalyzer(compiler).parse_file("test", source);
    auto ast = bonk::Parser(compiler).parse_file(&lexemes);

    ASSERT_TRUE(compiler.state == bonk::BONK_COMPILER_OK);
    ASSERT_NE(ast, nullptr);

    auto program = (bonk::TreeNodeProgram*)ast.get();
    ASSERT_EQ(program->body.size(), 1);

    auto blok_definition = (bonk::TreeNodeBlockDefinition*)program->body.front().get();
    ASSERT_EQ(blok_definition->body->body.size(), 6);

    std::vector<bonk::TreeNodeBinaryOperation*> binary_ops;

    for (auto& statement : blok_definition->body->body) {
        if (&statement == &blok_definition->body->body.front())
            continue;
        ASSERT_EQ(statement->type, bonk::TreeNodeType::n_binary_operation);
        binary_ops.push_back((bonk::TreeNodeBinaryOperation*)statement.get());
    }

    // a = (1 + (2 * 3))
    auto binary_op = binary_ops[0];
    ASSERT_EQ(binary_op->operator_type, bonk::OperatorType::o_assign);
    ASSERT_EQ(binary_op->left->type, bonk::TreeNodeType::n_identifier);
    ASSERT_EQ(binary_op->right->type, bonk::TreeNodeType::n_binary_operation);

    // 1 + (2 * 3)
    binary_op = (bonk::TreeNodeBinaryOperation*)binary_op->right.get();
    ASSERT_EQ(binary_op->operator_type, bonk::OperatorType::o_plus);
    ASSERT_EQ(binary_op->left->type, bonk::TreeNodeType::n_number_constant);
    ASSERT_EQ(binary_op->right->type, bonk::TreeNodeType::n_binary_operation);

    // 2 * 3
    binary_op = (bonk::TreeNodeBinaryOperation*)binary_op->right.get();
    ASSERT_EQ(binary_op->operator_type, bonk::OperatorType::o_multiply);
    ASSERT_EQ(binary_op->left->type, bonk::TreeNodeType::n_number_constant);
    ASSERT_EQ(binary_op->right->type, bonk::TreeNodeType::n_number_constant);

    // a = (1 * 2) and 3)
    binary_op = binary_ops[1];
    ASSERT_EQ(binary_op->operator_type, bonk::OperatorType::o_assign);
    ASSERT_EQ(binary_op->left->type, bonk::TreeNodeType::n_identifier);
    ASSERT_EQ(binary_op->right->type, bonk::TreeNodeType::n_binary_operation);

    // (1 * 2) and 3
    binary_op = (bonk::TreeNodeBinaryOperation*)binary_op->right.get();
    ASSERT_EQ(binary_op->operator_type, bonk::OperatorType::o_and);
    ASSERT_EQ(binary_op->left->type, bonk::TreeNodeType::n_binary_operation);
    ASSERT_EQ(binary_op->right->type, bonk::TreeNodeType::n_number_constant);

    // 1 * 2
    binary_op = (bonk::TreeNodeBinaryOperation*)binary_op->left.get();
    ASSERT_EQ(binary_op->operator_type, bonk::OperatorType::o_multiply);
    ASSERT_EQ(binary_op->left->type, bonk::TreeNodeType::n_number_constant);
    ASSERT_EQ(binary_op->right->type, bonk::TreeNodeType::n_number_constant);

    // a = (1 and 2) or 3
    binary_op = binary_ops[2];
    ASSERT_EQ(binary_op->operator_type, bonk::OperatorType::o_assign);
    ASSERT_EQ(binary_op->left->type, bonk::TreeNodeType::n_identifier);
    ASSERT_EQ(binary_op->right->type, bonk::TreeNodeType::n_binary_operation);

    // (1 and 2) or 3
    binary_op = (bonk::TreeNodeBinaryOperation*)binary_op->right.get();
    ASSERT_EQ(binary_op->operator_type, bonk::OperatorType::o_or);
    ASSERT_EQ(binary_op->left->type, bonk::TreeNodeType::n_binary_operation);
    ASSERT_EQ(binary_op->right->type, bonk::TreeNodeType::n_number_constant);

    // 1 and 2
    binary_op = (bonk::TreeNodeBinaryOperation*)binary_op->left.get();
    ASSERT_EQ(binary_op->operator_type, bonk::OperatorType::o_and);
    ASSERT_EQ(binary_op->left->type, bonk::TreeNodeType::n_number_constant);
    ASSERT_EQ(binary_op->right->type, bonk::TreeNodeType::n_number_constant);

    // a += (2 * 3)
    binary_op = binary_ops[3];
    ASSERT_EQ(binary_op->operator_type, bonk::OperatorType::o_plus_assign);
    ASSERT_EQ(binary_op->left->type, bonk::TreeNodeType::n_identifier);
    ASSERT_EQ(binary_op->right->type, bonk::TreeNodeType::n_binary_operation);

    // 2 * 3
    binary_op = (bonk::TreeNodeBinaryOperation*)binary_op->right.get();
    ASSERT_EQ(binary_op->operator_type, bonk::OperatorType::o_multiply);
    ASSERT_EQ(binary_op->left->type, bonk::TreeNodeType::n_number_constant);
    ASSERT_EQ(binary_op->right->type, bonk::TreeNodeType::n_number_constant);
}