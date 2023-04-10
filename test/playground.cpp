
#include <gtest/gtest.h>
#include "bonk/frontend/converters/hir_early_generator_visitor.hpp"
#include "bonk/frontend/frontend.hpp"
#include "bonk/frontend/parsing/lexic/lexer.hpp"
#include "bonk/frontend/parsing/parser.hpp"
#include "bonk/frontend/ast/ast_printer.hpp"
#include "bonk/middleend/ir/hir.hpp"
#include "bonk/middleend/ir/hir_printer.hpp"

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
    ASSERT_FALSE(lexemes.empty());

    auto root = bonk::Parser(compiler).parse_file(&lexemes);
    ASSERT_NE(root, nullptr);

    auto ast = bonk::AST();
    ast.root = std::move(root);

    bonk::FrontEnd front_end(compiler);

    ASSERT_EQ(front_end.transform_ast(ast), true);

    // Print AST
    bonk::ASTPrinter ast_printer{output_stream};
    ast.root->accept(&ast_printer);

    auto ir_program = bonk::HIREarlyGeneratorVisitor(front_end).generate(ast.root.get());

    ASSERT_NE(ir_program, nullptr);

    // Print IR

    bonk::HIRPrinter printer{output_stream};
    printer.print(*ir_program);
}