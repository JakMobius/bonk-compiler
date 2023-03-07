
#include <gtest/gtest.h>
#include "bonk/parsing/parser.hpp"
#include "bonk/tree/json_dump_ast_visitor.hpp"

TEST(Parser, TestHive) {
    auto error_stream = bonk::StdOutputStream(std::cout);
    auto out_stream = bonk::StdOutputStream(std::cout);

    bonk::CompilerConfig config{.error_file = error_stream};
    bonk::Compiler compiler(config);

    const char* source = R"(
        hive TestHive {
           bowl test_bowl: flot;
           bowl test_bowl2: nubr;
           bowl test_bowl3: strg;
           bowl test_bowl4: many strg;
        }
    )";

    auto lexemes = compiler.lexical_analyzer.parse_file("test", source);
    auto ast = compiler.parser.parse_file(&lexemes);

    ASSERT_TRUE(compiler.state == bonk::BONK_COMPILER_OK);

    // Print AST as a json

    bonk::JsonSerializer serializer{out_stream};
    bonk::JsonDumpAstVisitor visitor{serializer};

    ast->accept(&visitor);
}