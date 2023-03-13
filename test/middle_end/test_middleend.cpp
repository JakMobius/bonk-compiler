
#include <sstream>
#include <gtest/gtest.h>
#include "bonk/middleend/annotators/basic_symbol_annotator.hpp"
#include "bonk/middleend/converters/hive_constructor_call_replacer.hpp"
#include "bonk/middleend/converters/hive_ctor_dtor_early_generator.hpp"
#include "bonk/middleend/converters/stdlib_header_generator.hpp"
#include "bonk/middleend/ir/hir.hpp"
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
            bowl x = 1.0 + 2.0;
            x *= "hey";
        }
    )";

    auto lexemes = bonk::LexicalAnalyzer(compiler).parse_file("test", source);
    auto ast = bonk::Parser(compiler).parse_file(&lexemes);

    ASSERT_NE(ast, nullptr);

    bonk::MiddleEnd middle_end(compiler);

    EXPECT_EQ(middle_end.transform_ast(ast.get()), false);

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

    EXPECT_EQ(middle_end.transform_ast(ast.get()), false);
    EXPECT_EQ(error_stringstream.str(),
              "test:11:41: error: Cannot perform '+' between TestHive and nubr\n");
}

TEST(MiddleEnd, TypecheckerFibonacciTest) {
    // This test is aimed at testing the type checker's ability to correctly
    // infer the return type of recursive functions. The fibonacci function
    // has two bonk statements, one of which is a recursive call. The type checker
    // should start a nested type inference process with an assumption that the
    // fibonacci blok never returns, and infer the return type from the nested
    // process.

    auto error_stream = bonk::StdOutputStream(std::cerr);

    bonk::CompilerConfig config{.error_file = error_stream};
    bonk::Compiler compiler(config);

    const char* source = R"(
        blok fibonacci[bowl x: nubr] {
            x < 2 and { bonk x; };
            bonk @fibonacci[x = x - 1] + @fibonacci[x = x - 2];
        }
    )";

    auto lexemes = bonk::LexicalAnalyzer(compiler).parse_file("test", source);
    auto ast = bonk::Parser(compiler).parse_file(&lexemes);

    ASSERT_NE(ast, nullptr);

    bonk::MiddleEnd middle_end(compiler);
    EXPECT_EQ(middle_end.transform_ast(ast.get()), true);

    // Get the global scope and find the fibonacci function definition
    auto global_scope = middle_end.symbol_table.symbol_scopes[ast.get()];
    auto fibonacci_definition = global_scope->symbols["fibonacci"];

    auto fibonacci_block_type = middle_end.type_table.get_type(fibonacci_definition);
    auto fibonacci_return_type = ((bonk::BlokType*)fibonacci_block_type)->return_type.get();

    auto fibonacci_return_type_data = (bonk::TrivialType*)fibonacci_return_type;
    ASSERT_EQ(fibonacci_return_type_data->primitive_type, bonk::PrimitiveType::t_nubr);
}

TEST(MiddleEnd, TypecheckerRecursiveTest) {

    auto error_stream = bonk::StdOutputStream(std::cerr);

    bonk::CompilerConfig config{.error_file = error_stream};
    bonk::Compiler compiler(config);

    const char* source = R"(
        blok recursive_a[bowl x: nubr] {
            bonk @recursive_b[x = x] or @recursive_a[x = x - 1];
        }
        blok recursive_b[bowl x: nubr] {
            bonk x == 0;
        }
    )";

    auto lexemes = bonk::LexicalAnalyzer(compiler).parse_file("test", source);
    auto ast = bonk::Parser(compiler).parse_file(&lexemes);

    ASSERT_NE(ast, nullptr);

    bonk::MiddleEnd middle_end(compiler);
    EXPECT_EQ(middle_end.transform_ast(ast.get()), true);

    // Get the global scope and find the recursive_a and recursive_b function definition
    auto global_scope = middle_end.symbol_table.symbol_scopes[ast.get()];
    auto rec_a_definition = global_scope->symbols["recursive_a"];
    auto rec_b_definition = global_scope->symbols["recursive_b"];

    auto rec_a_block_type = middle_end.type_table.get_type(rec_a_definition);
    auto rec_b_block_type = middle_end.type_table.get_type(rec_b_definition);

    auto rec_a_return_type = ((bonk::BlokType*)rec_a_block_type)->return_type.get();
    auto rec_b_return_type = ((bonk::BlokType*)rec_b_block_type)->return_type.get();

    auto rec_a_return_type_data = (bonk::TrivialType*)rec_a_return_type;
    auto rec_b_return_type_data = (bonk::TrivialType*)rec_b_return_type;

    ASSERT_EQ(rec_a_return_type_data->primitive_type, bonk::PrimitiveType::t_buul);
    ASSERT_EQ(rec_b_return_type_data->primitive_type, bonk::PrimitiveType::t_buul);
}

TEST(MiddleEnd, TypecheckerRecursiveNeverTest) {

    auto error_stream = bonk::StdOutputStream(std::cerr);

    bonk::CompilerConfig config{.error_file = error_stream};
    bonk::Compiler compiler(config);

    const char* source = R"(
        blok recursive_a[bowl x: flot] {
            bonk @recursive_b[x = x];
        }
        blok recursive_b[bowl x: flot] {
            bonk @recursive_a[x = x];
        }
    )";

    auto lexemes = bonk::LexicalAnalyzer(compiler).parse_file("test", source);
    auto ast = bonk::Parser(compiler).parse_file(&lexemes);

    ASSERT_NE(ast, nullptr);

    bonk::MiddleEnd middle_end(compiler);
    EXPECT_EQ(middle_end.transform_ast(ast.get()), true);

    // Get the global scope and find the recursive_a and recursive_b function definition
    auto global_scope = middle_end.symbol_table.symbol_scopes[ast.get()];
    auto rec_a_definition = global_scope->symbols["recursive_a"];
    auto rec_b_definition = global_scope->symbols["recursive_b"];

    auto rec_a_block_type = middle_end.type_table.get_type(rec_a_definition);
    auto rec_b_block_type = middle_end.type_table.get_type(rec_b_definition);

    auto rec_a_return_type = ((bonk::BlokType*)rec_a_block_type)->return_type.get();
    auto rec_b_return_type = ((bonk::BlokType*)rec_b_block_type)->return_type.get();

    ASSERT_EQ(rec_a_return_type->kind, bonk::TypeKind::never);
    ASSERT_EQ(rec_b_return_type->kind, bonk::TypeKind::never);
}

TEST(MiddleEnd, TypecheckerFallproofTest) {

    // This test checks that the type-checker doesn't crash if return type of a blok
    // cannot be inferred due to a semantic error

    std::stringstream error_stringstream;
    auto error_stream = bonk::StdOutputStream(error_stringstream);

    bonk::CompilerConfig config{.error_file = error_stream};
    bonk::Compiler compiler(config);

    const char* source = R"(
        blok illegal_operation_1[bowl x: flot] {
            bonk @illegal_operation_2[x = "blob"];
        }
        blok illegal_operation_2[bowl x: flot] {
            bonk x == "blob";
        }
    )";

    auto lexemes = bonk::LexicalAnalyzer(compiler).parse_file("test", source);
    auto ast = bonk::Parser(compiler).parse_file(&lexemes);

    ASSERT_NE(ast, nullptr);

    bonk::MiddleEnd middle_end(compiler);
    EXPECT_EQ(middle_end.transform_ast(ast.get()), false);

    std::string error_string = error_stringstream.str();

    EXPECT_NE(error_string.find("Cannot perform '==' between flot and strg"), std::string::npos);
    EXPECT_NE(error_string.find("Cannot pass 'strg' to parameter 'x' of type 'flot'"),
              std::string::npos);
}

TEST(MiddleEnd, CodegenTest) {
    auto error_stream = bonk::StdOutputStream(std::cout);

    bonk::CompilerConfig config{.error_file = error_stream};
    bonk::Compiler compiler(config);

    const char* source = R"(
        blok add_one[bowl x: flot] {
            bonk x + 1.0;
        }

        blok main {
            @add_one[x = 1.0 + 5.0];
        }
    )";

    auto lexemes = bonk::LexicalAnalyzer(compiler).parse_file("test", source);
    auto ast = bonk::Parser(compiler).parse_file(&lexemes);

    ASSERT_NE(ast, nullptr);

    bonk::MiddleEnd middle_end(compiler);

    ASSERT_EQ(middle_end.transform_ast(ast.get()), true);
    auto ir_program = middle_end.generate_hir(ast.get());

    ASSERT_NE(ir_program, nullptr);

    std::vector<std::string> procedure_names_to_find = {
        "add_one",
        "main",
    };

    for (const auto& procedure : ir_program->procedures) {
        auto procedure_header =
            (bonk::HIRInstruction*)procedure.base_blocks[0].instructions.front();
        ASSERT_EQ(procedure_header->type, bonk::HIRInstructionType::procedure);
        auto procedure_id = ((bonk::HIRProcedure*)procedure_header)->procedure_id;
        auto procedure_definition = middle_end.id_table.get_node(procedure_id);

        ASSERT_EQ(procedure_definition->type, bonk::TreeNodeType::n_block_definition);
        auto& procedure_name =
            ((bonk::TreeNodeBlockDefinition*)procedure_definition)->block_name->identifier_text;

        auto it = std::find(procedure_names_to_find.begin(), procedure_names_to_find.end(),
                            procedure_name);

        if (it != procedure_names_to_find.end()) {
            procedure_names_to_find.erase(it);
        }
    }

    if (!procedure_names_to_find.empty()) {
        for (const auto& procedure_name : procedure_names_to_find) {
            FAIL() << procedure_name << " not found in IR program\n";
        }
        std::cout << std::endl;
    }
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
    EXPECT_NE(ast_string.find("blok $$bonk_object_inc_reference[bowl object: nubr]"),
              std::string::npos);
    EXPECT_NE(ast_string.find("blok $$bonk_object_dec_reference[bowl object: nubr]"),
              std::string::npos);
}

TEST(MiddleEnd, ConstructorDestructorGeneratorTest) {
    auto error_stream = bonk::StdOutputStream(std::cout);

    bonk::CompilerConfig config{.error_file = error_stream};
    bonk::Compiler compiler(config);

    const char* source = R"(
        hive Dummy {}

        hive TestHive {
            bowl x: flot = 1.5;
            bowl y: strg;
            bowl z: Dummy;
        }

        blok main {
            bowl my_hive_type = TestHive;
            bowl my_hive_instance: TestHive = @TestHive[y = "hey", z = @Dummy];
        }
    )";

    auto lexemes = bonk::LexicalAnalyzer(compiler).parse_file("test", source);
    auto ast = bonk::Parser(compiler).parse_file(&lexemes);

    ASSERT_NE(ast, nullptr);

    bonk::MiddleEnd middle_end(compiler);
    middle_end.transform_ast(ast.get());

    ASSERT_EQ(compiler.state, bonk::BONK_COMPILER_OK);

    // Make sure the constructor was generated
    EXPECT_EQ(ast->type, bonk::TreeNodeType::n_program);

    auto global_scope = middle_end.symbol_table.symbol_scopes[ast.get()];
    auto constructor =
        (bonk::TreeNodeBlockDefinition*)global_scope->symbols["TestHive$$constructor"];
    auto destructor = (bonk::TreeNodeBlockDefinition*)global_scope->symbols["TestHive$$destructor"];

    // Make sure the constructor has the correct signature
    EXPECT_EQ(constructor->block_name->identifier_text, "TestHive$$constructor");
    EXPECT_EQ(constructor->block_parameters->parameters.size(), 3);
    {
        auto it = constructor->block_parameters->parameters.begin();
        EXPECT_EQ((*it)->variable_name->identifier_text, "x");
        EXPECT_EQ((*it)->variable_type->type, bonk::TreeNodeType::n_primitive_type);
        EXPECT_EQ(((bonk::TreeNodePrimitiveType*)(*it)->variable_type.get())->primitive_type,
                  bonk::PrimitiveType::t_flot);
        EXPECT_EQ((*it)->variable_value->type, bonk::TreeNodeType::n_number_constant);
        EXPECT_NEAR(
            ((bonk::TreeNodeNumberConstant*)(*it)->variable_value.get())->contents.double_value,
            1.5, 1e-10);

        it++;
        EXPECT_EQ((*it)->variable_name->identifier_text, "y");
        EXPECT_EQ((*it)->variable_type->type, bonk::TreeNodeType::n_primitive_type);
        EXPECT_EQ(((bonk::TreeNodePrimitiveType*)(*it)->variable_type.get())->primitive_type,
                  bonk::PrimitiveType::t_strg);
        EXPECT_EQ((*it)->variable_value, nullptr);

        it++;
        EXPECT_EQ((*it)->variable_name->identifier_text, "z");
        EXPECT_EQ((*it)->variable_type->type, bonk::TreeNodeType::n_identifier);
        EXPECT_EQ((*it)->variable_value, nullptr);
        EXPECT_EQ(((bonk::TreeNodeIdentifier*)(*it)->variable_type.get())->identifier_text,
                  "Dummy");
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
        EXPECT_EQ(((bonk::TreeNodeNumberConstant*)parameter->parameter_value.get())
                      ->contents.integer_value,
                  24);
    }

    // Make sure the destructor has the correct signature
    EXPECT_EQ(destructor->block_name->identifier_text, "TestHive$$destructor");
    EXPECT_EQ(destructor->block_parameters->parameters.size(), 1);
    {
        auto it = destructor->block_parameters->parameters.begin();
        EXPECT_EQ((*it)->variable_name->identifier_text, "object");
        EXPECT_EQ((*it)->variable_type->type, bonk::TreeNodeType::n_identifier);
        EXPECT_EQ(((bonk::TreeNodeIdentifier*)(*it)->variable_type.get())->identifier_text,
                  "TestHive");
    }

    // Count the number of calls to the constructor
    struct IdentifierVisitor : bonk::ASTVisitor {
        int constructor_reference_count = 0;
        int hive_reference_count = 0;
        void visit(bonk::TreeNodeIdentifier* identifier) override {
            if (identifier->identifier_text == "TestHive$$constructor") {
                constructor_reference_count++;
            } else if (identifier->identifier_text == "TestHive") {
                hive_reference_count++;
            }
        }
    } visitor;

    ast->accept(&visitor);

    // Make sure that the replacer replaced all the identifiers
    // used in value context.

    // <compiler-generated definition of TestHive$$constructor> <- 1
    // bowl hive_type = TestHive; <- 2
    // bowl hive_instance: (...) = @TestHive; <- 3
    EXPECT_EQ(visitor.constructor_reference_count, 3);

    // Make sure that the replacer didn't replace what it shouldn't have
    // <compiler-generated definition of TestHive$$destructor> <- 1 (has one in its prototype)
    // hive TestHive <- 2
    // bowl hive_instance: TestHive = (...) <- 3
    EXPECT_EQ(visitor.hive_reference_count, 3);

    std::stringstream ast_stringstream;
    bonk::StdOutputStream stream{ast_stringstream};
    bonk::ASTPrinter printer{stream};
    ast->accept(&printer);
    std::string ast_string = ast_stringstream.str();
    std::cout << ast_string;
}
