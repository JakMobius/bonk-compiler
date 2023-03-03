
#include <gtest/gtest.h>
#include "bonk/parsing/parser.hpp"

TEST(Grammatics, TestHive) {
    auto error_stream = bonk::StdOutputStream(std::cerr);
    bonk::CompilerConfig config{
        .error_file = error_stream
    };
    bonk::Compiler compiler(config);

    const char* source = R"(
        hive TestHive {
           bowl test_bowl: flot;
           bowl test_bowl2: nubr;
           bowl test_bowl3: strg;
           bowl test_bowl4: many strg;

           blok test_blok {
               test_bowl of me = 1.0;
               test_bowl2 of me = 2;
               test_bowl3 of me = "test";
               test_bowl4 of me = ["test", "test2"];

               test_bowl of me > 0.5 and {
                   test_bowl of me = 0.0;
               } or {
                   test_bowl of me = 1.0;
               }

               loop[bowl counter = 0] {
                   test_bowl of me *= 2.0;
                   counter += 1 < 5 or brek;
               }
           }
        }
    )";


    auto lexemes = compiler.lexical_analyzer.parse_file("test", source);

    ASSERT_FALSE(lexemes.empty());
    ASSERT_TRUE(compiler.state == bonk::BONK_COMPILER_OK);
}
