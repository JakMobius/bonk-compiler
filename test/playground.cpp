
#include <gtest/gtest.h>
#include "bonk/frontend/ast/ast_printer.hpp"
#include "bonk/frontend/converters/hir_early_generator_visitor.hpp"
#include "bonk/frontend/frontend.hpp"
#include "bonk/frontend/parsing/lexic/lexer.hpp"
#include "bonk/frontend/parsing/parser.hpp"
#include "bonk/middleend/ir/algorithms/hir_dominator_finder.hpp"
#include "bonk/middleend/ir/hir.hpp"
#include "bonk/middleend/ir/hir_graphviz_dumper.hpp"
#include "bonk/middleend/ir/hir_printer.hpp"
#include "bonk/middleend/middleend.hpp"

TEST(Playground, Playground) {
    auto error_stream = bonk::StdOutputStream(std::cerr);
    auto output_stream = bonk::StdOutputStream(std::cout);

    bonk::CompilerConfig config{.error_file = error_stream};
    bonk::Compiler compiler(config);

    const char* source = R"(
        blok main {
            loop[bowl counter = 10] {
                counter = counter - 1;
                counter > 0 or { brek; };
            }
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
    //    bonk::ASTPrinter ast_printer{output_stream};
    //    ast.root->accept(&ast_printer);

    auto ir_program = front_end.generate_hir(ast.root.get());
    ASSERT_NE(ir_program, nullptr);

    // Print IR

    //    bonk::HIRPrinter printer{output_stream};
    //    printer.print(*ir_program);

    ASSERT_EQ(bonk::MiddleEnd(compiler).do_passes(*ir_program), true);

    // Print CFG

    bonk::HIRGraphvizDumper dumper{output_stream};
    dumper.dump(*ir_program);

    // Print dominators

    bonk::HIRDominatorFinder dominator_finder;
    auto dominators = dominator_finder.find_dominators(*ir_program->procedures[0]);

    for (int i = 0; i < dominators.size(); i++) {
        std::cout << "dominators[" << i << "] = {";
        for (int j = 0; j < dominators[i].size(); j++) {
            if (j != 0) {
                std::cout << ", ";
            }
            std::cout << (dominators[i][j] ? "1" : "0");
        }
        std::cout << "}" << std::endl;
    }
}