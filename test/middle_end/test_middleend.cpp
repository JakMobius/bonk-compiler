
#include <gtest/gtest.h>
#include "bonk/frontend/ast/ast_printer.hpp"
#include "bonk/frontend/frontend.hpp"
#include "bonk/middleend/ir/algorithms/dominance_frontier_finder.hpp"
#include "bonk/middleend/ir/algorithms/hir_dominator_finder.hpp"
#include "bonk/middleend/middleend.hpp"

TEST(MiddleEnd, DominatorListTest) {
    auto error_stream = bonk::StdOutputStream(std::cerr);
    auto output_stream = bonk::StdOutputStream(std::cout);

    bonk::CompilerConfig config{.error_file = error_stream};
    bonk::Compiler compiler(config);

    bonk::FrontEnd front_end(compiler);
    bonk::IDTable id_table(front_end);
    bonk::SymbolTable symbol_table;
    auto ir_program = std::make_unique<bonk::HIRProgram>(id_table, symbol_table);

    ir_program->create_procedure();
    auto& procedure = ir_program->procedures[0];
    procedure->create_base_block();
    auto& block = procedure->base_blocks[0];

    /*
     * L0:
     *  jmpnz %0, L1, L2
     * L1:
     *  jmp L3
     * L2:
     *  jmp L4
     * L3:
     *  jmp L0
     * L4:
     *  ret
     */

    block->instructions = {
        block->instruction<bonk::HIRLabelInstruction>(0),
        block->instruction<bonk::HIRJumpNZInstruction>(0, 1, 2),
        block->instruction<bonk::HIRLabelInstruction>(1),
        block->instruction<bonk::HIRJumpInstruction>(3),
        block->instruction<bonk::HIRLabelInstruction>(2),
        block->instruction<bonk::HIRJumpInstruction>(4),
        block->instruction<bonk::HIRLabelInstruction>(3),
        block->instruction<bonk::HIRJumpInstruction>(0),
        block->instruction<bonk::HIRLabelInstruction>(4),
        block->instruction<bonk::HIRReturnInstruction>(),
    };

//    bonk::HIRPrinter printer{output_stream};
//    printer.print(*ir_program);

    bonk::MiddleEnd middle_end(compiler);
    middle_end.program = std::move(ir_program);
    ASSERT_EQ(middle_end.do_passes(), true);

    bonk::HIRDominatorFinder dominator_finder;
    auto& dominators = dominator_finder.find_dominators(*middle_end.program->procedures[0]);

    ASSERT_EQ(dominators[0], std::vector<bool>({1, 0, 0, 0, 0, 0, 0}));
    ASSERT_EQ(dominators[1], std::vector<bool>({1, 1, 0, 0, 0, 0, 0}));
    ASSERT_EQ(dominators[2], std::vector<bool>({1, 1, 1, 0, 0, 0, 0}));
    ASSERT_EQ(dominators[3], std::vector<bool>({1, 1, 0, 1, 0, 0, 0}));
    ASSERT_EQ(dominators[4], std::vector<bool>({1, 1, 1, 0, 1, 0, 0}));
    ASSERT_EQ(dominators[5], std::vector<bool>({1, 1, 0, 1, 0, 1, 0}));
    ASSERT_EQ(dominators[6], std::vector<bool>({1, 1, 0, 1, 0, 1, 1}));
}

TEST(MiddleEnd, DominanceFrontierTest) {
    auto error_stream = bonk::StdOutputStream(std::cerr);
    auto output_stream = bonk::StdOutputStream(std::cout);

    bonk::CompilerConfig config{.error_file = error_stream};
    bonk::Compiler compiler(config);

    bonk::FrontEnd front_end(compiler);
    bonk::IDTable id_table(front_end);
    bonk::SymbolTable symbol_table;
    auto ir_program = std::make_unique<bonk::HIRProgram>(id_table, symbol_table);

    ir_program->create_procedure();
    auto& procedure = ir_program->procedures[0];
    procedure->create_base_block();
    auto& block = procedure->base_blocks[0];

    /*
     * L0:
     *  jmpnz %0, L1, L2
     * L1:
     *  jmp L3
     * L2:
     *  jmp L4
     * L3:
     *  jmp L0
     * L4:
     *  ret
     */

    block->instructions = {
        block->instruction<bonk::HIRLabelInstruction>(0),
        block->instruction<bonk::HIRJumpNZInstruction>(0, 1, 2),
        block->instruction<bonk::HIRLabelInstruction>(1),
        block->instruction<bonk::HIRJumpInstruction>(3),
        block->instruction<bonk::HIRLabelInstruction>(2),
        block->instruction<bonk::HIRJumpInstruction>(4),
        block->instruction<bonk::HIRLabelInstruction>(3),
        block->instruction<bonk::HIRJumpInstruction>(0),
        block->instruction<bonk::HIRLabelInstruction>(4),
        block->instruction<bonk::HIRReturnInstruction>(),
    };

//    bonk::HIRPrinter printer{output_stream};
//    printer.print(*ir_program);

    bonk::MiddleEnd middle_end(compiler);
    middle_end.program = std::move(ir_program);
    ASSERT_EQ(middle_end.do_passes(), true);

    bonk::HIRDominanceFrontierFinder dominance_frontier_finder;
    auto& dominance_frontiers = dominance_frontier_finder.find_dominators(*middle_end.program->procedures[0]);

    ASSERT_EQ(dominance_frontiers, std::vector<int>({-1, 1, 1, -1, 1, -1, -1}));
}