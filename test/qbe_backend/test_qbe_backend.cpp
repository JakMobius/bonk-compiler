
#include <sstream>
#include <gtest/gtest.h>
#include "bonk/backend/qbe/qbe_backend.hpp"
#include "bonk/middleend/middleend.hpp"
#include "bonk/parsing/parser.hpp"
#include "bonk/tree/json_dump_ast_visitor.hpp"

TEST(QBEBackend, CodegenTest) {

    std::stringstream result_stream;

    auto error_stream = bonk::StdOutputStream(std::cout);
    auto output_stream = bonk::StdOutputStream(result_stream);

    bonk::CompilerConfig config{.error_file = error_stream, .output_file = output_stream};
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

    bonk::qbe_backend::QBEBackend backend{compiler};
    backend.compile_program(*ir_program);

    std::cout << result_stream.str() << std::endl;
}