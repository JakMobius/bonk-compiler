
#include <sstream>
#include <gtest/gtest.h>
#include "bonk/backend/qbe/qbe_backend.hpp"
#include "bonk/middleend/middleend.hpp"
#include "bonk/parsing/parser.hpp"

TEST(QBEBackend, CodegenTest) {

    std::stringstream result_stream;

    auto error_stream = bonk::StdOutputStream(std::cout);
    auto output_stream = bonk::StdOutputStream(result_stream);

    bonk::CompilerConfig config{.error_file = error_stream, .output_file = output_stream};
    bonk::Compiler compiler(config);

    const char* source = R"(
        blok add_one[bowl x: flot] {
            bonk x + 1.0;
        }

        blok main {
            @add_one[x = 1.0 + 5.0];
        }
    )";

    auto lexemes = bonk::Lexer(compiler).parse_file("test", source);
    auto ast = bonk::Parser(compiler).parse_file(&lexemes);

    ASSERT_NE(ast, nullptr);

    bonk::MiddleEnd middle_end(compiler);

    middle_end.transform_ast(ast.get());
    auto ir_program = middle_end.generate_hir(ast.get());

    bonk::qbe_backend::QBEBackend backend{compiler};
    backend.compile_program(*ir_program);
    std::string result = result_stream.str();

    // add_one function definition
    ASSERT_NE(result.find("export function s $\"_add_one\" (s "), std::string::npos);

    // main function definition
    ASSERT_NE(result.find("export function w $\"_main\" () {"), std::string::npos);

    // add_one function call
    ASSERT_NE(result.find(" =s call $\"_add_one\"(s "), std::string::npos);

    // make sure there are s_1 and s_5 constants
    ASSERT_NE(result.find("s_1"), std::string::npos);
    ASSERT_NE(result.find("s_5"), std::string::npos);
}