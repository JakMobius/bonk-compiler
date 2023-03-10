
#include <sstream>
#include <gtest/gtest.h>
#include "bonk/middleend/middleend.hpp"
#include "bonk/parsing/parser.hpp"
#include "bonk/tree/json_dump_ast_visitor.hpp"

TEST(MiddleEnd, TypecheckerTest1) {
    std::stringstream error_stringstream;
    auto error_stream = bonk::StdOutputStream(error_stringstream);

    bonk::CompilerConfig config{.error_file = error_stream};
    bonk::Compiler compiler(config);

    const char* source = R"(
        blok test_call {
            bowl x = 1 + 2;
            x *= "hey";
        }
    )";

    auto lexemes = compiler.lexical_analyzer.parse_file("test", source);
    auto ast = compiler.parser.parse_file(&lexemes);

    ASSERT_NE(ast, nullptr);

    bonk::MiddleEnd middle_end(compiler);

    EXPECT_EQ(middle_end.run_ast(ast.get()), false);
    EXPECT_EQ(error_stringstream.str(),
              "test:4:17: error: Cannot perform '*=' between flot and strg\n");
}

TEST(MiddleEnd, TypecheckerTest2) {
    std::stringstream error_stringstream;
    auto error_stream = bonk::StdOutputStream(error_stringstream);

    bonk::CompilerConfig config{.error_file = error_stream};
    bonk::Compiler compiler(config);

    const char* source = R"(
        hive TestHive {
            blok copy {
                bonk @TestHive;
            }
            blok test {}
        }

        blok test_call {
            bowl my_hive = @TestHive;
            @test of (@copy of my_hive + 1);
        }
    )";

    auto lexemes = compiler.lexical_analyzer.parse_file("test", source);
    auto ast = compiler.parser.parse_file(&lexemes);

    ASSERT_NE(ast, nullptr);

    bonk::MiddleEnd middle_end(compiler);

    EXPECT_EQ(middle_end.run_ast(ast.get()), false);
    EXPECT_EQ(error_stringstream.str(),
              "test:11:41: error: Cannot perform '+' between TestHive and flot\n");
}

TEST(MiddleEnd, CodegenTest) {
    auto error_stream = bonk::StdOutputStream(std::cout);

    bonk::CompilerConfig config{.error_file = error_stream};
    bonk::Compiler compiler(config);

    const char* source = R"(
        blok add_one[bowl x: flot] {
            bonk x + 1;
        }

        blok main {
            @add_one[x = 1 + 5];
        }
    )";

    auto lexemes = compiler.lexical_analyzer.parse_file("test", source);
    auto ast = compiler.parser.parse_file(&lexemes);

    ASSERT_NE(ast, nullptr);

    bonk::MiddleEnd middle_end(compiler);

    middle_end.run_ast(ast.get());
}