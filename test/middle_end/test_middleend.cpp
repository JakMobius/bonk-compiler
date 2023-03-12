
#include <sstream>
#include <gtest/gtest.h>
#include "bonk/middleend/converters/hive_constructor_generator.hpp"
#include "bonk/middleend/converters/stdlib_header_generator.hpp"
#include "bonk/middleend/middleend.hpp"
#include "bonk/parsing/parser.hpp"
#include "bonk/tree/ast_printer.hpp"
#include "bonk/tree/json_dump_ast_visitor.hpp"

TEST(MiddleEnd, TypecheckerTest1) {
    std::stringstream error_stringstream;
    auto error_stream = bonk::StdOutputStream(error_stringstream);

    bonk::CompilerConfig config{.error_file = error_stream};
    bonk::Compiler compiler(config);

    const char* source = R"(
        blok test_call {
            bowl x = 1 + 2;
            x *= "hey";
        }
    )";

    auto lexemes = bonk::LexicalAnalyzer(compiler).parse_file("test", source);
    auto ast = bonk::Parser(compiler).parse_file(&lexemes);

    ASSERT_NE(ast, nullptr);

    bonk::MiddleEnd middle_end(compiler);

    EXPECT_EQ(middle_end.run_ast(ast.get()), nullptr);
    EXPECT_EQ(error_stringstream.str(),
              "test:4:17: error: Cannot perform '*=' between flot and strg\n");
}

TEST(MiddleEnd, TypecheckerTest2) {
    std::stringstream error_stringstream;
    auto error_stream = bonk::StdOutputStream(error_stringstream);

    bonk::CompilerConfig config{.error_file = error_stream};
    bonk::Compiler compiler(config);

    const char* source = R"(
        hive TestHive {
            blok copy {
                bonk @TestHive;
            }
            blok test {}
        }

        blok test_call {
            bowl my_hive = @TestHive;
            @test of (@copy of my_hive + 1);
        }
    )";

    auto lexemes = bonk::LexicalAnalyzer(compiler).parse_file("test", source);
    auto ast = bonk::Parser(compiler).parse_file(&lexemes);

    ASSERT_NE(ast, nullptr);

    bonk::MiddleEnd middle_end(compiler);

    EXPECT_EQ(middle_end.run_ast(ast.get()), nullptr);
    EXPECT_EQ(error_stringstream.str(),
              "test:11:41: error: Cannot perform '+' between TestHive and flot\n");
}

TEST(MiddleEnd, CodegenTest) {
    auto error_stream = bonk::StdOutputStream(std::cout);

    bonk::CompilerConfig config{.error_file = error_stream};
    bonk::Compiler compiler(config);

    const char* source = R"(
        blok add_one[bowl x: flot] {
            bonk x + 1;
        }

        blok main {
            @add_one[x = 1 + 5];
        }
    )";

    auto lexemes = bonk::LexicalAnalyzer(compiler).parse_file("test", source);
    auto ast = bonk::Parser(compiler).parse_file(&lexemes);

    ASSERT_NE(ast, nullptr);

    bonk::MiddleEnd middle_end(compiler);

    auto ir_program = middle_end.run_ast(ast.get());

    EXPECT_EQ(ir_program->procedures.size(), 2);
}

TEST(MiddleEnd, StdLibHeaderGenerator) {
    auto error_stream = bonk::StdOutputStream(std::cout);

    bonk::CompilerConfig config{.error_file = error_stream};
    bonk::Compiler compiler(config);

    const char* source = R"(
        blok main {}
    )";

    auto lexemes = bonk::LexicalAnalyzer(compiler).parse_file("test", source);
    auto ast = bonk::Parser(compiler).parse_file(&lexemes);

    ASSERT_NE(ast, nullptr);

    bonk::MiddleEnd middle_end(compiler);
    bonk::StdLibHeaderGenerator header_generator(middle_end);

    header_generator.generate(ast.get());

    std::stringstream ast_stringstream;
    bonk::StdOutputStream stream{ast_stringstream};
    bonk::ASTPrinter printer{stream};
    ast->accept(&printer);

    std::string ast_string = ast_stringstream.str();

    EXPECT_NE(ast_string.find("blok $$bonk_create_object[bowl size: nubr]"), std::string::npos);
    EXPECT_NE(ast_string.find("blok $$bonk_object_free[bowl object: nubr]"), std::string::npos);
    EXPECT_NE(ast_string.find("blok $$bonk_object_inc_reference[bowl object: nubr]"), std::string::npos);
    EXPECT_NE(ast_string.find("blok $$bonk_object_dec_reference[bowl object: nubr]"), std::string::npos);
}

TEST(MiddleEnd, ConstructorGeneratorTest) {
    auto error_stream = bonk::StdOutputStream(std::cout);

    bonk::CompilerConfig config{.error_file = error_stream};
    bonk::Compiler compiler(config);

    const char* source = R"(
        hive TestHive {
            bowl x: flot = 1.5;
            bowl y: strg;
            bowl z: many nubr;
        }
    )";

    auto lexemes = bonk::LexicalAnalyzer(compiler).parse_file("test", source);
    auto ast = bonk::Parser(compiler).parse_file(&lexemes);

    ASSERT_NE(ast, nullptr);

    bonk::MiddleEnd middle_end(compiler);
    bonk::HiveConstructorGenerator constructor_generator(middle_end);

    constructor_generator.generate(ast.get());

    // Make sure the constructor was generated
    EXPECT_EQ(ast->type, bonk::TreeNodeType::n_program);
    auto program = (bonk::TreeNodeProgram*)(ast.get());
    EXPECT_EQ(program->body.size(), 2);
    EXPECT_EQ(program->body.front()->type, bonk::TreeNodeType::n_hive_definition);
    EXPECT_EQ(program->body.back()->type, bonk::TreeNodeType::n_block_definition);

    // Make sure the constructor has the correct signature
    auto constructor = (bonk::TreeNodeBlockDefinition*)program->body.back().get();
    EXPECT_EQ(constructor->block_name->identifier_text, "TestHive$$constructor");
    EXPECT_EQ(constructor->block_parameters->parameters.size(), 3);
    {
        auto it = constructor->block_parameters->parameters.begin();
        EXPECT_EQ((*it)->variable_name->identifier_text, "x");
        EXPECT_EQ((*it)->variable_type->type, bonk::TreeNodeType::n_primitive_type);
        EXPECT_EQ(((bonk::TreeNodePrimitiveType*)(*it)->variable_type.get())->primitive_type,
                  bonk::PrimitiveType::t_flot);
        EXPECT_EQ((*it)->variable_value->type, bonk::TreeNodeType::n_number_constant);
        EXPECT_NEAR(((bonk::TreeNodeNumberConstant*)(*it)->variable_value.get())->double_value, 1.5,
                    1e-10);

        it++;
        EXPECT_EQ((*it)->variable_name->identifier_text, "y");
        EXPECT_EQ((*it)->variable_type->type, bonk::TreeNodeType::n_primitive_type);
        EXPECT_EQ(((bonk::TreeNodePrimitiveType*)(*it)->variable_type.get())->primitive_type,
                  bonk::PrimitiveType::t_strg);
        EXPECT_EQ((*it)->variable_value, nullptr);

        it++;
        EXPECT_EQ((*it)->variable_name->identifier_text, "z");
        EXPECT_EQ((*it)->variable_type->type, bonk::TreeNodeType::n_many_type);
        EXPECT_EQ(((bonk::TreeNodeManyType*)(*it)->variable_type.get())->parameter->type,
                  bonk::TreeNodeType::n_primitive_type);
        EXPECT_EQ((*it)->variable_value, nullptr);
    }

    {
        // Check the constructor body
        auto body = constructor->body.get();
        auto it = body->body.begin();

        // Variable definition, three assignments, and a return
        EXPECT_EQ(body->body.size(), 5);

        EXPECT_EQ((*it)->type, bonk::TreeNodeType::n_variable_definition);

        auto variable_definition = (bonk::TreeNodeVariableDefinition*)(*it).get();
        EXPECT_EQ(variable_definition->variable_name->identifier_text, "object");
        EXPECT_EQ(variable_definition->variable_value->type, bonk::TreeNodeType::n_call);

        auto call = (bonk::TreeNodeCall*)(variable_definition->variable_value.get());
        EXPECT_EQ(middle_end.type_table.get_type(call)->kind, bonk::TypeKind::hive);
        EXPECT_EQ(call->callee->type, bonk::TreeNodeType::n_identifier);
        EXPECT_EQ(((bonk::TreeNodeIdentifier*)call->callee.get())->identifier_text,
                  "$$bonk_create_object");
        EXPECT_EQ(call->arguments->parameters.size(), 1);

        auto parameter = call->arguments->parameters.front().get();
        EXPECT_EQ(parameter->parameter_name->identifier_text, "size");
        EXPECT_EQ(parameter->parameter_value->type, bonk::TreeNodeType::n_number_constant);
        EXPECT_EQ(((bonk::TreeNodeNumberConstant*)parameter->parameter_value.get())->integer_value,
                  20);
    }
}