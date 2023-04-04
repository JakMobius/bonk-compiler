
#include <gtest/gtest.h>
#include "bonk/middleend/ir/hir.hpp"
#include "bonk/middleend/ir/hir_early_generator_visitor.hpp"
#include "bonk/middleend/ir/hir_printer.hpp"
#include "bonk/middleend/middleend.hpp"
#include "bonk/parsing/lexic/lexer.hpp"
#include "bonk/parsing/parser.hpp"
#include "bonk/tree/ast_printer.hpp"

TEST(Playground, Playground) {
    auto error_stream = bonk::StdOutputStream(std::cerr);
    auto output_stream = bonk::StdOutputStream(std::cout);

    bonk::CompilerConfig config{.error_file = error_stream };
    bonk::Compiler compiler(config);

    const char* source = R"(
        blok main {

        }
    )";

    auto lexemes = bonk::Lexer(compiler).parse_file("test", source);
    auto root = bonk::Parser(compiler).parse_file(&lexemes);
    auto ast = bonk::AST();
    ast.root = std::move(root);

    ASSERT_NE(ast.root, nullptr);

    bonk::MiddleEnd middle_end(compiler);

    ASSERT_EQ(middle_end.transform_ast(ast), true);

    // Print AST
    bonk::ASTPrinter ast_printer{output_stream};
    ast.root->accept(&ast_printer);

    auto ir_program = bonk::HIREarlyGeneratorVisitor(middle_end).generate(ast.root.get());

    ASSERT_NE(ir_program, nullptr);

    // Print IR

    bonk::HIRPrinter printer{output_stream};
    printer.print(*ir_program);
}