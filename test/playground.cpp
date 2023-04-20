
#include <gtest/gtest.h>
#include "bonk/frontend/ast/ast_printer.hpp"
#include "bonk/frontend/converters/hir_early_generator_visitor.hpp"
#include "bonk/frontend/frontend.hpp"
#include "bonk/frontend/parsing/lexic/lexer.hpp"
#include "bonk/frontend/parsing/parser.hpp"
#include "bonk/middleend/ir/algorithms/hir_base_block_separator.hpp"
#include "bonk/middleend/ir/algorithms/hir_copy_propagation.hpp"
#include "bonk/middleend/ir/algorithms/hir_dominator_finder.hpp"
#include "bonk/middleend/ir/algorithms/hir_loc_collapser.hpp"
#include "bonk/middleend/ir/algorithms/hir_ref_count_replacer.hpp"
#include "bonk/middleend/ir/algorithms/hir_ssa_converter.hpp"
#include "bonk/middleend/ir/algorithms/hir_unreachable_code_deleter.hpp"
#include "bonk/middleend/ir/algorithms/hir_unused_def_deleter.hpp"
#include "bonk/middleend/ir/algorithms/hir_variable_index_compressor.hpp"
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
        hive Value {
            bowl value = 10;
        }

        hive Box {
            bowl box: Value = null;
        }

        blok main {
            bowl box = @Box;
            bowl box2 = box;
            bowl box3 = box2;
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

    bonk::HIRVariableIndexCompressor().compress(*ir_program);
    bonk::HIRBaseBlockSeparator().separate_blocks(*ir_program);
    bonk::HIRLocCollapser().collapse(*ir_program);

    // <optimizations>
    bonk::HIRSSAConverter().convert(*ir_program);
    bonk::HIRCopyPropagation().propagate_copies(*ir_program);
    bonk::HIRUnusedDefDeleter().delete_unused_defs(*ir_program);
    bonk::HIRUnreachableCodeDeleter().delete_unreachable_code(*ir_program);
    bonk::HIRVariableIndexCompressor().compress(*ir_program);
    // </optimizations>

//    bonk::HIRRefCountReplacer().replace_ref_counters(*ir_program);

    bonk::HIRPrinter printer{output_stream};
    printer.print(*ir_program);

    // Print CFG

//    bonk::HIRGraphvizDumper dumper{output_stream};
//    dumper.dump(*ir_program);

    // Print dominators

    bonk::HIRDominatorFinder dominator_finder(*ir_program->procedures[0]);
    auto dominators = dominator_finder.get_dominators();

//    for (int i = 0; i < dominators.size(); i++) {
//        std::cout << "dominators[" << i << "] = {";
//        for (int j = 0; j < dominators[i].size(); j++) {
//            if (j != 0) {
//                std::cout << ", ";
//            }
//            std::cout << (dominators[i][j] ? "1" : "0");
//        }
//        std::cout << "}" << std::endl;
//    }
}