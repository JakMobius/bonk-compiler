
#include <gtest/gtest.h>
#include "bonk/frontend/ast/ast_printer.hpp"
#include "bonk/frontend/frontend.hpp"
#include "bonk/middleend/ir/algorithms/hir_alive_variables_finder.hpp"
#include "bonk/middleend/ir/algorithms/hir_base_block_separator.hpp"
#include "bonk/middleend/ir/algorithms/hir_copy_propagation.hpp"
#include "bonk/middleend/ir/algorithms/hir_dominance_frontier_finder.hpp"
#include "bonk/middleend/ir/algorithms/hir_dominator_finder.hpp"
#include "bonk/middleend/ir/algorithms/hir_ssa_converter.hpp"
#include "bonk/middleend/ir/algorithms/hir_unreachable_code_deleter.hpp"
#include "bonk/middleend/ir/algorithms/hir_unused_def_deleter.hpp"
#include "bonk/middleend/ir/algorithms/hir_variable_index_compressor.hpp"
#include "bonk/middleend/ir/hir_graphviz_dumper.hpp"
#include "bonk/middleend/middleend.hpp"

TEST(MiddleEnd, DominatorListTest1) {
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

    bonk::HIRBaseBlockSeparator().separate_blocks(*ir_program);

    bonk::HIRDominatorFinder dominator_finder(*ir_program->procedures[0]);
    auto dominators = dominator_finder.get_dominators();

    ASSERT_EQ(dominators[0], bonk::DynamicBitSet({1, 0, 0, 0, 0, 0, 0}));
    ASSERT_EQ(dominators[1], bonk::DynamicBitSet({1, 1, 0, 0, 0, 0, 0}));
    ASSERT_EQ(dominators[2], bonk::DynamicBitSet({1, 1, 1, 0, 0, 0, 0}));
    ASSERT_EQ(dominators[3], bonk::DynamicBitSet({1, 1, 0, 1, 0, 0, 0}));
    ASSERT_EQ(dominators[4], bonk::DynamicBitSet({1, 1, 1, 0, 1, 0, 0}));
    ASSERT_EQ(dominators[5], bonk::DynamicBitSet({1, 1, 0, 1, 0, 1, 0}));
    ASSERT_EQ(dominators[6], bonk::DynamicBitSet({1, 1, 0, 1, 0, 1, 1}));
}

TEST(MiddleEnd, DominatorListTest2) {
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

    block->instructions = {
        block->instruction<bonk::HIRLabelInstruction>(0),
        block->instruction<bonk::HIRConstantLoadInstruction>(0, (int64_t)5),
        block->instruction<bonk::HIRConstantLoadInstruction>(1, (int64_t)10),
        block->instruction<bonk::HIRConstantLoadInstruction>(2, (int64_t)15),
        block->instruction<bonk::HIRJumpNZInstruction>(0, 1, 2),

        block->instruction<bonk::HIRLabelInstruction>(1),
        block->instruction<bonk::HIRMemoryLoadInstruction>(2, 1, bonk::HIRDataType::dword),
        block->instruction<bonk::HIRJumpInstruction>(3),

        block->instruction<bonk::HIRLabelInstruction>(2),
        block->instruction<bonk::HIRMemoryLoadInstruction>(1, 0, bonk::HIRDataType::dword),
        block->instruction<bonk::HIRJumpInstruction>(3),

        block->instruction<bonk::HIRLabelInstruction>(3),
        block->instruction<bonk::HIRMemoryLoadInstruction>(0, 0, bonk::HIRDataType::dword),
        block->instruction<bonk::HIRMemoryLoadInstruction>(1, 1, bonk::HIRDataType::dword),
        block->instruction<bonk::HIRMemoryLoadInstruction>(2, 2, bonk::HIRDataType::dword),
        block->instruction<bonk::HIRReturnInstruction>(),
    };

    bonk::HIRBaseBlockSeparator().separate_blocks(*ir_program);

    bonk::HIRDominatorFinder dominator_finder(*ir_program->procedures[0]);
    auto dominators = dominator_finder.get_dominators();

    // print dominators
    //    for (int i = 0; i < dominators.size(); i++) {
    //        std::cout << "Block " << i << ": ";
    //        for (int j = 0; j < dominators.size(); j++) {
    //            if (dominators[i][j]) {
    //                std::cout << j << " ";
    //            }
    //        }
    //        std::cout << std::endl;
    //    }

    ASSERT_EQ(dominators[0], bonk::DynamicBitSet({1, 0, 0, 0, 0, 0}));
    ASSERT_EQ(dominators[1], bonk::DynamicBitSet({1, 1, 0, 0, 0, 0}));
    ASSERT_EQ(dominators[2], bonk::DynamicBitSet({1, 1, 1, 0, 0, 0}));
    ASSERT_EQ(dominators[3], bonk::DynamicBitSet({1, 1, 0, 1, 0, 0}));
    ASSERT_EQ(dominators[4], bonk::DynamicBitSet({1, 1, 0, 0, 1, 0}));
    ASSERT_EQ(dominators[5], bonk::DynamicBitSet({1, 1, 0, 0, 1, 1}));
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
    auto& ir_procedure = ir_program->procedures[0];
    ir_procedure->create_base_block();
    auto& block = ir_procedure->base_blocks[0];

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

    bonk::HIRBaseBlockSeparator().separate_blocks(*ir_program);

    bonk::HIRDominatorFinder d_finder(*ir_procedure);
    bonk::HIRDominanceTreeBuilder dt_finder(d_finder);
    bonk::HIRDominanceFrontierFinder df_finder(dt_finder);

    auto dominance_frontiers = df_finder.get_frontiers();

    ASSERT_EQ(dominance_frontiers, std::vector<int>({-1, 1, 1, -1, 1, -1, -1}));
}

TEST(MiddleEnd, AliveVariablesTest) {
    auto error_stream = bonk::StdOutputStream(std::cerr);
    auto output_stream = bonk::StdOutputStream(std::cout);

    bonk::CompilerConfig config{.error_file = error_stream};
    bonk::Compiler compiler(config);

    bonk::FrontEnd front_end(compiler);
    bonk::IDTable id_table(front_end);
    bonk::SymbolTable symbol_table;
    auto ir_program = std::make_unique<bonk::HIRProgram>(id_table, symbol_table);

    ir_program->create_procedure();
    auto& ir_procedure = ir_program->procedures[0];
    ir_procedure->create_base_block();
    auto& block = ir_procedure->base_blocks[0];

    /*
     * L0:
     *  %0 <- 0
     *  %1 <- 0
     *  jmp L1
     * L1:
     *  jmp L2
     * L2:
     *  %1 <- load %0 ; %0 is alive
     *  %2 <- load %1 ; %1 is not alive, because it is overwritten
     *  jmp L3
     * L3:
     *  ret
     */

    block->instructions = {
        block->instruction<bonk::HIRLabelInstruction>(0),
        block->instruction<bonk::HIRConstantLoadInstruction>(0, (int64_t)0),
        block->instruction<bonk::HIRConstantLoadInstruction>(1, (int64_t)0),
        block->instruction<bonk::HIRJumpInstruction>(1),

        block->instruction<bonk::HIRLabelInstruction>(1),
        block->instruction<bonk::HIRJumpInstruction>(2),

        block->instruction<bonk::HIRLabelInstruction>(2),
        block->instruction<bonk::HIRMemoryLoadInstruction>(1, 0, bonk::HIRDataType::dword),
        block->instruction<bonk::HIRMemoryLoadInstruction>(2, 1, bonk::HIRDataType::dword),
        block->instruction<bonk::HIRJumpInstruction>(3),

        block->instruction<bonk::HIRLabelInstruction>(3),
        block->instruction<bonk::HIRReturnInstruction>(),
    };

    // HIRVariableIndexCompressor should be called before HIRAliveVariablesFinder
    // because it uses procedure.used_variables, which is filled by HIRVariableIndexCompressor
    bonk::HIRVariableIndexCompressor().compress(*ir_program);
    bonk::HIRBaseBlockSeparator().separate_blocks(*ir_program);

    auto& procedure = *ir_program->procedures[0];
    bonk::HIRAliveVariablesFinder av_finder;
    av_finder.walk(procedure);

    bonk::DynamicBitSet expected_ins[] = {{0, 0, 0}, {0, 0, 0}, {1, 0, 0},
                                          {1, 0, 0}, {0, 0, 0}, {0, 0, 0}};

    bonk::DynamicBitSet expected_outs[] = {{0, 0, 0}, {1, 0, 0}, {1, 0, 0},
                                           {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};

    bonk::DynamicBitSet expected_uses[] = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0},
                                           {1, 0, 0}, {0, 0, 0}, {0, 0, 0}};

    bonk::DynamicBitSet expected_defs[] = {{0, 0, 0}, {1, 1, 0}, {0, 0, 0},
                                           {0, 1, 1}, {0, 0, 0}, {0, 0, 0}};

    for (int i = 0; i < procedure.base_blocks.size(); i++) {
        ASSERT_EQ(av_finder.get_in(*procedure.base_blocks[i]), expected_ins[i]);
        ASSERT_EQ(av_finder.out[i], expected_outs[i]);
        ASSERT_EQ(av_finder.use[i], expected_uses[i]);
        ASSERT_EQ(av_finder.define[i], expected_defs[i]);
    }
}

TEST(MiddleEnd, VariableIndexCompressorTest) {
    auto error_stream = bonk::StdOutputStream(std::cerr);
    auto output_stream = bonk::StdOutputStream(std::cout);

    bonk::CompilerConfig config{.error_file = error_stream};
    bonk::Compiler compiler(config);

    bonk::FrontEnd front_end(compiler);
    bonk::IDTable id_table(front_end);
    bonk::SymbolTable symbol_table;
    auto ir_program = std::make_unique<bonk::HIRProgram>(id_table, symbol_table);

    ir_program->create_procedure();
    auto ir_procedure = ir_program->procedures[0].get();
    ir_procedure->create_base_block();
    auto block = ir_procedure->base_blocks[0].get();

    /*
     * L0:
     *  %0 <- 0
     *  %1 <- 0
     *  jmp L1
     * L1:
     *  jmp L2
     * L2:
     *  %1 <- load %0 ; %0 is alive
     *  %2 <- load %1 ; %1 is not alive, because it is overwritten
     *  jmp L3
     * L3:
     *  ret
     */

    block->instructions = {
        block->instruction<bonk::HIRLabelInstruction>(0),
        block->instruction<bonk::HIRConstantLoadInstruction>(2, (int64_t)0),
        block->instruction<bonk::HIRConstantLoadInstruction>(4, (int64_t)0),
        block->instruction<bonk::HIRJumpInstruction>(1),

        block->instruction<bonk::HIRLabelInstruction>(1),
        block->instruction<bonk::HIRConstantLoadInstruction>(12, (int64_t)0),
        block->instruction<bonk::HIRJumpInstruction>(2),

        block->instruction<bonk::HIRLabelInstruction>(2),
        block->instruction<bonk::HIRReturnInstruction>(),
    };

    bonk::HIRBaseBlockSeparator().separate_blocks(*ir_program);
    bonk::HIRVariableIndexCompressor().compress(*ir_procedure);

    ASSERT_EQ(ir_procedure->used_registers, 3);

    auto b1_instructions = ir_procedure->base_blocks[1]->instructions.begin();
    auto b2_instructions = ir_procedure->base_blocks[2]->instructions.begin();

    // Don't increment the instruction iterators, because labels get omitted by the
    // block separator
    // ++b1_instructions;
    // ++b2_instructions;

    ASSERT_EQ((*b1_instructions)->get_write_register(0, nullptr), 0);
    ASSERT_EQ((*b2_instructions)->get_write_register(0, nullptr), 2);
}

TEST(MiddleEnd, SSAConverterTest1) {
    auto error_stream = bonk::StdOutputStream(std::cerr);
    auto output_stream = bonk::StdOutputStream(std::cout);

    bonk::CompilerConfig config{.error_file = error_stream};
    bonk::Compiler compiler(config);

    bonk::FrontEnd front_end(compiler);
    bonk::IDTable id_table(front_end);
    bonk::SymbolTable symbol_table;
    auto ir_program = std::make_unique<bonk::HIRProgram>(id_table, symbol_table);

    ir_program->create_procedure();
    auto& ir_procedure = ir_program->procedures[0];
    ir_procedure->create_base_block();
    auto& block = ir_procedure->base_blocks[0];

    block->instructions = {
        block->instruction<bonk::HIRLabelInstruction>(0),
        block->instruction<bonk::HIRConstantLoadInstruction>(0, (int64_t)0),
        block->instruction<bonk::HIRConstantLoadInstruction>(1, (int64_t)0),
        block->instruction<bonk::HIRJumpNZInstruction>(0, 1, 2),

        block->instruction<bonk::HIRLabelInstruction>(1),
        block->instruction<bonk::HIRConstantLoadInstruction>(0, (int64_t)1),
        block->instruction<bonk::HIRJumpInstruction>(2),

        block->instruction<bonk::HIRLabelInstruction>(2),
        block->instruction<bonk::HIRMemoryLoadInstruction>(1, 0, bonk::HIRDataType::dword),
        block->instruction<bonk::HIRMemoryLoadInstruction>(2, 1, bonk::HIRDataType::dword),
        block->instruction<bonk::HIRJumpInstruction>(3),

        block->instruction<bonk::HIRLabelInstruction>(3),
        block->instruction<bonk::HIRReturnInstruction>(),
    };

    bonk::HIRVariableIndexCompressor().compress(*ir_procedure);
    bonk::HIRBaseBlockSeparator().separate_blocks(*ir_program);
    bonk::HIRSSAConverter().convert(*ir_procedure);

    //    bonk::HIRPrinter(output_stream).print(*ir_program);

    std::unordered_set<bonk::IRRegister> used_variables;
    // Make sure all variables are only assigned once

    int phi_functions = 0;

    for (auto& block : ir_procedure->base_blocks) {
        for (auto& instruction : block->instructions) {
            if (instruction->type == bonk::HIRInstructionType::phi_function) {
                phi_functions++;
            }
            for (int i = 0; i < instruction->get_write_register_count(); i++) {
                auto variable = instruction->get_write_register(i, nullptr);
                ASSERT_TRUE(used_variables.find(variable) == used_variables.end());
                used_variables.insert(variable);
            }
        }
    }

    EXPECT_EQ(phi_functions, 1);
    EXPECT_EQ(ir_procedure->used_registers, used_variables.size());
}

TEST(MiddleEnd, SSAConverterTest2) {
    auto error_stream = bonk::StdOutputStream(std::cerr);
    auto output_stream = bonk::StdOutputStream(std::cout);

    bonk::CompilerConfig config{.error_file = error_stream};
    bonk::Compiler compiler(config);

    bonk::FrontEnd front_end(compiler);
    bonk::IDTable id_table(front_end);
    bonk::SymbolTable symbol_table;
    auto ir_program = std::make_unique<bonk::HIRProgram>(id_table, symbol_table);

    ir_program->create_procedure();
    auto& ir_procedure = ir_program->procedures[0];
    ir_procedure->create_base_block();
    auto& block = ir_procedure->base_blocks[0];

    block->instructions = {
        block->instruction<bonk::HIRLabelInstruction>(0),
        block->instruction<bonk::HIRConstantLoadInstruction>(0, (int64_t)5),
        block->instruction<bonk::HIRConstantLoadInstruction>(1, (int64_t)10),
        block->instruction<bonk::HIRConstantLoadInstruction>(2, (int64_t)15),
        block->instruction<bonk::HIRJumpNZInstruction>(0, 1, 2),

        block->instruction<bonk::HIRLabelInstruction>(1),
        block->instruction<bonk::HIRMemoryLoadInstruction>(2, 1, bonk::HIRDataType::dword),
        block->instruction<bonk::HIRJumpInstruction>(3),

        block->instruction<bonk::HIRLabelInstruction>(2),
        block->instruction<bonk::HIRMemoryLoadInstruction>(1, 0, bonk::HIRDataType::dword),
        block->instruction<bonk::HIRJumpInstruction>(3),

        block->instruction<bonk::HIRLabelInstruction>(3),
        block->instruction<bonk::HIRMemoryLoadInstruction>(0, 0, bonk::HIRDataType::dword),
        block->instruction<bonk::HIRMemoryLoadInstruction>(1, 1, bonk::HIRDataType::dword),
        block->instruction<bonk::HIRMemoryLoadInstruction>(2, 2, bonk::HIRDataType::dword),
        block->instruction<bonk::HIRReturnInstruction>(),
    };

    bonk::HIRVariableIndexCompressor().compress(*ir_procedure);
    bonk::HIRBaseBlockSeparator().separate_blocks(*ir_program);

    bonk::HIRSSAConverter().convert(*ir_procedure);

    std::unordered_set<bonk::IRRegister> used_variables;
    // Make sure all variables are only assigned once

    int phi_functions = 0;

    for (auto& block : ir_procedure->base_blocks) {
        for (auto& instruction : block->instructions) {
            if (instruction->type == bonk::HIRInstructionType::phi_function) {
                phi_functions++;
            }
            for (int i = 0; i < instruction->get_write_register_count(); i++) {
                auto variable = instruction->get_write_register(i, nullptr);
                ASSERT_TRUE(used_variables.find(variable) == used_variables.end());
                used_variables.insert(variable);
            }
        }
    }

    EXPECT_EQ(phi_functions, 2);
    EXPECT_EQ(ir_procedure->used_registers, used_variables.size());
}

TEST(MiddleEnd, UnreachableCodeDeleterTest) {
    auto error_stream = bonk::StdOutputStream(std::cerr);
    auto output_stream = bonk::StdOutputStream(std::cout);

    bonk::CompilerConfig config{.error_file = error_stream};
    bonk::Compiler compiler(config);

    bonk::FrontEnd front_end(compiler);
    bonk::IDTable id_table(front_end);
    bonk::SymbolTable symbol_table;
    auto ir_program = std::make_unique<bonk::HIRProgram>(id_table, symbol_table);

    ir_program->create_procedure();
    auto& ir_procedure = ir_program->procedures[0];
    ir_procedure->create_base_block();
    auto& block = ir_procedure->base_blocks[0];

    block->instructions = {
        block->instruction<bonk::HIRLabelInstruction>(0),
        block->instruction<bonk::HIRConstantLoadInstruction>(0, (int64_t)0),
        block->instruction<bonk::HIRConstantLoadInstruction>(1, (int64_t)0),
        block->instruction<bonk::HIRJumpNZInstruction>(0, 2, 3),

        // First unreachable block
        block->instruction<bonk::HIRLabelInstruction>(1),
        block->instruction<bonk::HIRConstantLoadInstruction>(0, (int64_t)1),
        block->instruction<bonk::HIRJumpInstruction>(3),

        block->instruction<bonk::HIRLabelInstruction>(2),
        block->instruction<bonk::HIRConstantLoadInstruction>(0, (int64_t)1),
        block->instruction<bonk::HIRJumpInstruction>(3),

        block->instruction<bonk::HIRLabelInstruction>(3),
        block->instruction<bonk::HIRMemoryLoadInstruction>(1, 0, bonk::HIRDataType::dword),
        block->instruction<bonk::HIRMemoryLoadInstruction>(2, 1, bonk::HIRDataType::dword),
        block->instruction<bonk::HIRReturnInstruction>(),

        // Second unreachable block
        block->instruction<bonk::HIRLabelInstruction>(4),
        block->instruction<bonk::HIRConstantLoadInstruction>(0, (int64_t)1),
        block->instruction<bonk::HIRJumpInstruction>(4),
    };

    bonk::HIRVariableIndexCompressor().compress(*ir_procedure);
    bonk::HIRBaseBlockSeparator().separate_blocks(*ir_program);

    int blocks = ir_procedure->base_blocks.size();

    bonk::HIRUnreachableCodeDeleter().delete_unreachable_code(*ir_procedure);

    EXPECT_EQ(ir_procedure->base_blocks.size(), blocks - 2);

    // Make sure blocks are in order
    for (int i = 0; i < ir_procedure->base_blocks.size(); i++) {
        EXPECT_EQ(ir_procedure->base_blocks[i]->index, i);
    }
}

TEST(MiddleEnd, CopyDistributionTest) {
    auto error_stream = bonk::StdOutputStream(std::cerr);
    auto output_stream = bonk::StdOutputStream(std::cout);

    bonk::CompilerConfig config{.error_file = error_stream};
    bonk::Compiler compiler(config);

    bonk::FrontEnd front_end(compiler);
    bonk::IDTable id_table(front_end);
    bonk::SymbolTable symbol_table;
    auto ir_program = std::make_unique<bonk::HIRProgram>(id_table, symbol_table);

    ir_program->create_procedure();
    auto& ir_procedure = ir_program->procedures[0];
    ir_procedure->create_base_block();
    auto& block = ir_procedure->base_blocks[0];

    block->instructions = {
        block->instruction<bonk::HIRLabelInstruction>(0),
        block->instruction<bonk::HIRConstantLoadInstruction>(0, (int64_t)0),
        block->instruction<bonk::HIRConstantLoadInstruction>(1, (int64_t)1),
        block->instruction<bonk::HIRMemoryLoadInstruction>(2, (int64_t)0, bonk::HIRDataType::dword),

        block->instruction<bonk::HIRJumpInstruction>(1),

        block->instruction<bonk::HIRLabelInstruction>(1),
        &block->instruction<bonk::HIROperationInstruction>()->set_assign(3, 0,
                                                                         bonk::HIRDataType::dword),
        &block->instruction<bonk::HIROperationInstruction>()->set_assign(4, 1,
                                                                         bonk::HIRDataType::dword),
        &block->instruction<bonk::HIROperationInstruction>()->set_assign(5, 2,
                                                                         bonk::HIRDataType::dword),
        block->instruction<bonk::HIRJumpInstruction>(2),

        block->instruction<bonk::HIRLabelInstruction>(2),
        &block->instruction<bonk::HIROperationInstruction>()->set_assign(6, 5,
                                                                         bonk::HIRDataType::dword),
        &block->instruction<bonk::HIROperationInstruction>()->set_assign(7, 4,
                                                                         bonk::HIRDataType::dword),
        &block->instruction<bonk::HIROperationInstruction>()->set_assign(8, 3,
                                                                         bonk::HIRDataType::dword),
        block->instruction<bonk::HIRJumpInstruction>(3),

        block->instruction<bonk::HIRLabelInstruction>(3),
        block->instruction<bonk::HIRReturnInstruction>(),
    };

    bonk::HIRVariableIndexCompressor().compress(*ir_procedure);
    bonk::HIRBaseBlockSeparator().separate_blocks(*ir_program);
    bonk::HIRCopyPropagation().propagate_copies(*ir_procedure);

    //    bonk::HIRPrinter printer(output_stream);
    //    printer.print(*ir_program);

    int loads = 0;
    int assigns = 0;

    for (auto& block : ir_procedure->base_blocks) {
        for (auto& instruction : block->instructions) {
            if (instruction->type == bonk::HIRInstructionType::operation) {
                auto operation = (bonk::HIROperationInstruction*)instruction;
                if (operation->operation_type == bonk::HIROperationType::assign) {
                    bonk::IRRegister target = operation->target;
                    bonk::IRRegister source = operation->left;

                    switch (target) {
                    case 5:
                        EXPECT_EQ(source, 2);
                        break;
                    case 6:
                        EXPECT_EQ(source, 2);
                        break;
                    default:
                        ADD_FAILURE() << "Unexpected register " << target;
                        break;
                    }

                    assigns++;
                }
            }

            if (instruction->type == bonk::HIRInstructionType::constant_load) {
                auto constant = (bonk::HIRConstantLoadInstruction*)instruction;
                bonk::IRRegister target = constant->target;
                int64_t value = constant->constant;

                switch (target) {
                case 0:
                    EXPECT_EQ(value, 0);
                    break;
                case 1:
                    EXPECT_EQ(value, 1);
                    break;
                case 3:
                    EXPECT_EQ(value, 0);
                    break;
                case 4:
                    EXPECT_EQ(value, 1);
                    break;
                case 7:
                    EXPECT_EQ(value, 1);
                    break;
                case 8:
                    EXPECT_EQ(value, 0);
                    break;
                default:
                    ADD_FAILURE() << "Unexpected register " << target;
                    break;
                }

                loads++;
            }
        }
    }

    EXPECT_EQ(loads, 6);
    EXPECT_EQ(assigns, 2);
}

TEST(MiddleEnd, UnusedDefinitionDeletionTest) {
    auto error_stream = bonk::StdOutputStream(std::cerr);
    auto output_stream = bonk::StdOutputStream(std::cout);

    bonk::CompilerConfig config{.error_file = error_stream};
    bonk::Compiler compiler(config);

    bonk::FrontEnd front_end(compiler);
    bonk::IDTable id_table(front_end);
    bonk::SymbolTable symbol_table;
    auto ir_program = std::make_unique<bonk::HIRProgram>(id_table, symbol_table);

    ir_program->create_procedure();
    auto& ir_procedure = ir_program->procedures[0];
    ir_procedure->create_base_block();
    auto& block = ir_procedure->base_blocks[0];

    block->instructions = {
        block->instruction<bonk::HIRLabelInstruction>(0),
        block->instruction<bonk::HIRConstantLoadInstruction>(0, (int64_t)0),
        &block->instruction<bonk::HIROperationInstruction>()->set_assign(1, 0,
                                                                         bonk::HIRDataType::dword),
        &block->instruction<bonk::HIROperationInstruction>()->set_assign(2, 1,
                                                                         bonk::HIRDataType::dword),
        &block->instruction<bonk::HIROperationInstruction>()->set_assign(3, 2,
                                                                         bonk::HIRDataType::dword),
        &block->instruction<bonk::HIROperationInstruction>()->set_assign(4, 3,
                                                                         bonk::HIRDataType::dword),
        &block->instruction<bonk::HIROperationInstruction>()->set_assign(5, 4,
                                                                         bonk::HIRDataType::dword),

        block->instruction<bonk::HIRReturnInstruction>(2),
    };

    bonk::HIRVariableIndexCompressor().compress(*ir_procedure);
    bonk::HIRBaseBlockSeparator().separate_blocks(*ir_program);

    bonk::HIRUnusedDefDeleter().delete_unused_defs(*ir_procedure);

    bonk::HIRPrinter printer(output_stream);
    printer.print(*ir_program);

    for (auto& block : ir_procedure->base_blocks) {
        for (auto& instruction : block->instructions) {
            if (instruction->type == bonk::HIRInstructionType::operation) {
                auto operation = (bonk::HIROperationInstruction*)instruction;
                ASSERT_LE(operation->target, 2);
                ASSERT_LE(operation->left, 1);
            }
        }
    }
}