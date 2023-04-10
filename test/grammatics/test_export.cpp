
#include <gtest/gtest.h>
#include "bonk/parsing/parser.hpp"
#include "bonk/tree/binary_ast_deserializer.hpp"
#include "bonk/tree/binary_ast_serializer.hpp"
#include "bonk/tree/json_ast_serializer.hpp"
#include "bonk/tree/template_visitor.hpp"
#include "utils/json_serializer.hpp"

std::string dump_ast_to_json(bonk::TreeNode* ast) {
    std::stringstream json_stream;
    bonk::StdOutputStream json_output{json_stream};
    bonk::JSONSerializer serializer{json_output};
    bonk::JSONASTSerializer ast_serializer{serializer};
    ast->accept(&ast_serializer);
    return json_stream.str();
}

TEST(Export, TestJSONExport) {
    auto error_stream = bonk::StdOutputStream(std::cerr);
    bonk::CompilerConfig config{.error_file = error_stream};
    bonk::Compiler compiler(config);

    const char* source = R"(
        hive TestHive {
           bowl test_bowl: many flot;

            blok test_blok[bowl counter = 0] {
                test_bowl of me = [1.0, 2.0, 3.0];
                test_bowl of me *= 2.0;
                counter += 1 < 5 or { brek; };
            }
        }
    )";

    auto lexemes = bonk::Lexer(compiler).parse_file("test", source);
    ASSERT_FALSE(lexemes.empty());

    auto ast = bonk::Parser(compiler).parse_file(&lexemes);
    ASSERT_NE(ast, nullptr);

//    std::stringstream json_stream;
//    bonk::StdOutputStream json_output{json_stream};
//    bonk::JSONSerializer serializer{json_output};
//    bonk::JSONASTSerializer ast_serializer{serializer};
//    ast->accept(&ast_serializer);
//    std::cout << json_stream.str() << std::endl;
}

TEST(Export, TestBinaryExport) {
    auto error_stream = bonk::StdOutputStream(std::cerr);
    bonk::CompilerConfig config{.error_file = error_stream};
    bonk::Compiler compiler(config);

    const char* source = R"(
        help "neighbour.bs"
        hive TestHive {
           bowl test_bowl: many flot;

            blok test_blok[bowl counter = 0] {
                test_bowl of me = [1.0, 2.0, 3.0];
                test_bowl of me *= 2.0;
                counter += 1 < 5 or { brek; };
                @test_blok[counter = counter];
                loop { brek; }
            }
        }
    )";

    auto lexemes = bonk::Lexer(compiler).parse_file("test", source);
    ASSERT_FALSE(lexemes.empty());

    auto ast = bonk::Parser(compiler).parse_file(&lexemes);
    ASSERT_NE(ast, nullptr);

    std::stringstream output_stream;
    bonk::StdOutputStream binary_output{output_stream};
    bonk::BinaryASTSerializer ast_serializer{binary_output};
    ast->accept(&ast_serializer);

    // Now decode it back
    std::string input = output_stream.str();
    bonk::BufferInputStream input_stream{input};
    bonk::BinaryASTDeserializer ast_deserializer{input_stream};
    auto decoded_ast = ast_deserializer.read();

    // Make sure it's the same
    ASSERT_EQ(dump_ast_to_json(ast.get()), dump_ast_to_json(decoded_ast.get()));
}
