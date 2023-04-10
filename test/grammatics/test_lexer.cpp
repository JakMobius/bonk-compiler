
#include <gtest/gtest.h>
#include "bonk/parsing/parser.hpp"

TEST(Lexer, TestHive) {
    auto error_stream = bonk::StdOutputStream(std::cerr);
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

    auto lexemes = bonk::Lexer(compiler).parse_file("test", source);
    ASSERT_FALSE(lexemes.empty());

    ASSERT_EQ(lexemes.size(), 26);
    EXPECT_TRUE(lexemes[0].is(bonk::OperatorType::o_hive));
    EXPECT_TRUE(lexemes[1].is_identifier("TestHive"));
    EXPECT_TRUE(lexemes[2].is(bonk::BraceType('{')));
    EXPECT_TRUE(lexemes[3].is(bonk::OperatorType::o_bowl));
    EXPECT_TRUE(lexemes[4].is_identifier("test_bowl"));
    EXPECT_TRUE(lexemes[5].type == bonk::LexemeType::l_colon);
    EXPECT_TRUE(lexemes[6].is(bonk::KeywordType::k_flot));
    EXPECT_TRUE(lexemes[7].type == bonk::LexemeType::l_semicolon);
    EXPECT_TRUE(lexemes[8].is(bonk::OperatorType::o_bowl));
    EXPECT_TRUE(lexemes[9].is_identifier("test_bowl2"));
    EXPECT_TRUE(lexemes[10].type == bonk::LexemeType::l_colon);
    EXPECT_TRUE(lexemes[11].is(bonk::KeywordType::k_nubr));
    EXPECT_TRUE(lexemes[12].type == bonk::LexemeType::l_semicolon);
    EXPECT_TRUE(lexemes[13].is(bonk::OperatorType::o_bowl));
    EXPECT_TRUE(lexemes[14].is_identifier("test_bowl3"));
    EXPECT_TRUE(lexemes[15].type == bonk::LexemeType::l_colon);
    EXPECT_TRUE(lexemes[16].is(bonk::KeywordType::k_strg));
    EXPECT_TRUE(lexemes[17].type == bonk::LexemeType::l_semicolon);
    EXPECT_TRUE(lexemes[18].is(bonk::OperatorType::o_bowl));
    EXPECT_TRUE(lexemes[19].is_identifier("test_bowl4"));
    EXPECT_TRUE(lexemes[20].type == bonk::LexemeType::l_colon);
    EXPECT_TRUE(lexemes[21].is(bonk::KeywordType::k_many));
    EXPECT_TRUE(lexemes[22].is(bonk::KeywordType::k_strg));
    EXPECT_TRUE(lexemes[23].type == bonk::LexemeType::l_semicolon);
    EXPECT_TRUE(lexemes[24].is(bonk::BraceType('}')));
    EXPECT_TRUE(lexemes[25].type == bonk::LexemeType::l_eof);
}

TEST(Lexer, TestComment) {
auto error_stream = bonk::StdOutputStream(std::cerr);
    bonk::CompilerConfig config{.error_file = error_stream};
    bonk::Compiler compiler(config);

    const char* source = R"(
        dogo: This is a comment
        "string with dogo: inside"
        -dogo-> This is a multiline comment <-dogo-
        "string with -dogo-> inside"
    )";

    auto lexemes = bonk::Lexer(compiler).parse_file("test", source);
    ASSERT_FALSE(lexemes.empty());

    ASSERT_EQ(lexemes.size(), 3);
    EXPECT_TRUE(lexemes[0].type == bonk::LexemeType::l_string);
    EXPECT_TRUE(lexemes[1].type == bonk::LexemeType::l_string);
    EXPECT_TRUE(lexemes[2].type == bonk::LexemeType::l_eof);
}

TEST(Lexer, TestComment2) {
        auto error_stream = bonk::StdOutputStream(std::cerr);
        bonk::CompilerConfig config{.error_file = error_stream};
        bonk::Compiler compiler(config);

        const char* source = R"(
            hive Person {
                bowl age -dogo-> how old   <-dogo- = 0;
            }
        )";

        auto lexemes = bonk::Lexer(compiler).parse_file("test", source);
        ASSERT_FALSE(lexemes.empty());

        ASSERT_EQ(lexemes.size(), 10);
        EXPECT_EQ(lexemes[0].type, bonk::LexemeType::l_operator);
        EXPECT_EQ(lexemes[1].type, bonk::LexemeType::l_identifier);
        EXPECT_EQ(lexemes[2].type, bonk::LexemeType::l_brace);
        EXPECT_EQ(lexemes[3].type, bonk::LexemeType::l_operator);
        EXPECT_EQ(lexemes[4].type, bonk::LexemeType::l_identifier);
        EXPECT_EQ(lexemes[5].type, bonk::LexemeType::l_operator);
        EXPECT_EQ(lexemes[6].type, bonk::LexemeType::l_number);
        EXPECT_EQ(lexemes[7].type, bonk::LexemeType::l_semicolon);
        EXPECT_EQ(lexemes[8].type, bonk::LexemeType::l_brace);
        EXPECT_EQ(lexemes[9].type, bonk::LexemeType::l_eof);
}

TEST(Lexer, TestStrings) {
    auto error_stream = bonk::StdOutputStream(std::cerr);
    bonk::CompilerConfig config{.error_file = error_stream};
    bonk::Compiler compiler(config);

    const char* source = R"(
        "test"
        "test\twith\ttabs"
        "test\nwith\nnewlines"
        "test\"with\"quotes"
        "test\\with\\backslashes"
    )";

    auto lexemes = bonk::Lexer(compiler).parse_file("test", source);
    ASSERT_FALSE(lexemes.empty());

    ASSERT_EQ(lexemes.size(), 6);
    EXPECT_TRUE(lexemes[0].is_string("test"));
    EXPECT_TRUE(lexemes[1].is_string("test\twith\ttabs"));
    EXPECT_TRUE(lexemes[2].is_string("test\nwith\nnewlines"));
    EXPECT_TRUE(lexemes[3].is_string("test\"with\"quotes"));
    EXPECT_TRUE(lexemes[4].is_string("test\\with\\backslashes"));
    EXPECT_TRUE(lexemes[5].type == bonk::LexemeType::l_eof);
}

TEST(Lexer, TestNumbers) {
    auto error_stream = bonk::StdOutputStream(std::cerr);
    bonk::CompilerConfig config{.error_file = error_stream};
    bonk::Compiler compiler(config);

    const char* source = R"(
        1
        1.0
        .05
        1e10
        1.0e-10
        1.0e+10
        .1E10
        .1E-10
        0x1
        0xFF
        0o1
        0o77
        0b1
        0b101010
    )";

    auto lexemes = bonk::Lexer(compiler).parse_file("test", source);
    ASSERT_FALSE(lexemes.empty());

    ASSERT_EQ(lexemes.size(), 15);
    for (int i = 0; i < 14; i++) {
        ASSERT_EQ(lexemes[i].type, bonk::LexemeType::l_number);
    }
    EXPECT_EQ(lexemes[14].type, bonk::LexemeType::l_eof);
    EXPECT_EQ(std::get<bonk::NumberLexeme>(lexemes[0].data).contents.integer_value, 1);
    EXPECT_EQ(std::get<bonk::NumberLexeme>(lexemes[0].data).contents.kind,
              bonk::NumberConstantKind::rather_integer);
    EXPECT_NEAR(std::get<bonk::NumberLexeme>(lexemes[0].data).contents.double_value, 1, 1e-20);
    EXPECT_NEAR(std::get<bonk::NumberLexeme>(lexemes[1].data).contents.double_value, 1, 1e-20);
    EXPECT_EQ(std::get<bonk::NumberLexeme>(lexemes[1].data).contents.kind,
              bonk::NumberConstantKind::rather_double);
    EXPECT_NEAR(std::get<bonk::NumberLexeme>(lexemes[2].data).contents.double_value, .05, 1e-10);
    EXPECT_NEAR(std::get<bonk::NumberLexeme>(lexemes[3].data).contents.double_value, 1e+10, 1);
    EXPECT_EQ(std::get<bonk::NumberLexeme>(lexemes[3].data).contents.kind,
              bonk::NumberConstantKind::rather_double);
    EXPECT_NEAR(std::get<bonk::NumberLexeme>(lexemes[4].data).contents.double_value, 1.0e-10,
                1e-20);
    EXPECT_NEAR(std::get<bonk::NumberLexeme>(lexemes[5].data).contents.double_value, 1.0e+10, 1);
    EXPECT_NEAR(std::get<bonk::NumberLexeme>(lexemes[6].data).contents.double_value, .1E10, 1);
    EXPECT_NEAR(std::get<bonk::NumberLexeme>(lexemes[7].data).contents.double_value, .1E-10, 1e-20);
    EXPECT_EQ(std::get<bonk::NumberLexeme>(lexemes[8].data).contents.integer_value, 1);
    EXPECT_EQ(std::get<bonk::NumberLexeme>(lexemes[9].data).contents.integer_value, 0xFF);
    EXPECT_EQ(std::get<bonk::NumberLexeme>(lexemes[10].data).contents.integer_value, 1);
    EXPECT_EQ(std::get<bonk::NumberLexeme>(lexemes[11].data).contents.integer_value, 7 + 7 * 8);
    EXPECT_EQ(std::get<bonk::NumberLexeme>(lexemes[12].data).contents.integer_value, 1);
    EXPECT_EQ(std::get<bonk::NumberLexeme>(lexemes[13].data).contents.integer_value, 0b101010);
    EXPECT_EQ(std::get<bonk::NumberLexeme>(lexemes[13].data).contents.kind,
              bonk::NumberConstantKind::rather_integer);
}

TEST(Lexer, TestGlobal) {
    auto error_stream = bonk::StdOutputStream(std::cerr);
    bonk::CompilerConfig config{.error_file = error_stream};
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

    auto lexemes = bonk::Lexer(compiler).parse_file("test", source);
    ASSERT_FALSE(lexemes.empty());
}

TEST(Lexer, TestOperators) {
    auto error_stream = bonk::StdOutputStream(std::cerr);
    bonk::CompilerConfig config{.error_file = error_stream};
    bonk::Compiler compiler(config);

    std::stringstream source_stream;

    for (int i = 0; bonk::BONK_OPERATOR_NAMES[i]; i++) {
        source_stream << bonk::BONK_OPERATOR_NAMES[i] << " ";
    }

    std::string source = source_stream.str();

    auto lexemes = bonk::Lexer(compiler).parse_file("test", source);
    ASSERT_FALSE(lexemes.empty());

    for (int i = 0; i < lexemes.size() - 1; i++) {
        ASSERT_EQ(lexemes[i].type, bonk::LexemeType::l_operator);
        EXPECT_EQ(std::get<bonk::OperatorLexeme>(lexemes[i].data).type, (bonk::OperatorType)i)
            << "Operator " << bonk::BONK_OPERATOR_NAMES[i] << " is not parsed correctly";
    }
}