
#include <sstream>
#include <gtest/gtest.h>
#include "bonk/backend/qbe/qbe_backend.hpp"
#include "bonk/frontend/frontend.hpp"
#include "bonk/frontend/parsing/parser.hpp"
#include "bonk/middleend/middleend.hpp"
#include "bonk/middleend/ir/algorithms/hir_ssa_converter.hpp"

TEST(QBEBackend, CodegenTest) {

    std::stringstream result_stream;

    auto error_stream = bonk::StdOutputStream(std::cout);
    auto output_stream = bonk::StdOutputStream(result_stream);

    bonk::CompilerConfig config{.error_file = error_stream };
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
    ASSERT_FALSE(lexemes.empty());

    auto root = bonk::Parser(compiler).parse_file(&lexemes);
    ASSERT_NE(root, nullptr);

    auto ast = bonk::AST();
    ast.root = std::move(root);

    bonk::FrontEnd front_end(compiler);

    ASSERT_TRUE(front_end.transform_ast(ast));
    auto ir_program = front_end.generate_hir(ast.root.get());

    ASSERT_NE(ir_program, nullptr);

    ASSERT_TRUE(bonk::MiddleEnd(compiler).do_passes(*ir_program));

    bonk::qbe_backend::QBEBackend backend{compiler};
    backend.compile_program(*ir_program, output_stream);
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