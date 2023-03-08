
#include <gtest/gtest.h>
#include "bonk/middleend/middleend.hpp"
#include "bonk/parsing/parser.hpp"
#include "bonk/tree/json_dump_ast_visitor.hpp"

TEST(MiddleEnd, SimpleTest) {
    auto error_stream = bonk::StdOutputStream(std::cout);

    bonk::CompilerConfig config{.error_file = error_stream};
    bonk::Compiler compiler(config);

    const char* source = R"(
        hive TestHive {
            blok copy {
                bonk @TestHive;
            }
            blok test_blok[bowl param: strg] {}
        }

        blok test_call {
            bowl my_hive = @TestHive;
            @test_blok of (@copy of my_hive + 1);

            bowl x = 1 + 2;
            x *= "hey";

            @test_blok2 of (@copy of my_hive);
        }
    )";

    auto lexemes = compiler.lexical_analyzer.parse_file("test", source);
    auto ast = compiler.parser.parse_file(&lexemes);

    bonk::MiddleEnd middle_end(compiler);

    middle_end.run_ast(ast.get());

}