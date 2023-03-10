
#include <array>
#include <gtest/gtest.h>
#include "utils.hpp"

TEST(TestQBEFullCycle, TestHiveConstruct) {
    const char* bonk_source = R"(
        hive TestHive {
            bowl test_field1: flot = 100.0;
            bowl test_field2: flot = 300.0;
        }

        blok bonk_main {
            bonk @TestHive[test_field1 = 1.0];
        }
    )";

    const char* c_source = R"(
        #include <stdio.h>

        struct TestHive {
            float test_field1;
            float test_field2;
        };

        struct TestHive* bonk_main();

        int main() {
            struct TestHive* bonk_response = bonk_main();
            printf("%f %f",
                   bonk_response->test_field1,
                   bonk_response->test_field2);
        }
    )";

    ASSERT_TRUE(run_bonk_with_counterpart(bonk_source, c_source, "test"));
    EXPECT_EQ(run_executable("test"), "1.000000 300.000000");
}

TEST(TestQBEFullCycle, TestHiveAssign) {
    const char* bonk_source = R"(
        hive TestHive {
            bowl test_field1: flot = 100.0;
            bowl test_field2: flot = 300.0;
        }

        blok bonk_main {
            bowl my_hive = @TestHive;
            test_field1 of my_hive = 1.0;
            test_field2 of my_hive = 2.0;
            bonk my_hive;
        }
    )";

    const char* c_source = R"(
        #include <stdio.h>

        struct TestHive {
            float test_field1;
            float test_field2;
        };

        struct TestHive* bonk_main();

        int main() {
            struct TestHive* bonk_response = bonk_main();
            printf("%f %f",
                   bonk_response->test_field1,
                   bonk_response->test_field2);
        }
    )";

    ASSERT_TRUE(run_bonk_with_counterpart(bonk_source, c_source, "test"));
    EXPECT_EQ(run_executable("test"), "1.000000 2.000000");
}

TEST(TestQBEFullCycle, TestHiveComplex) {
    const char* bonk_source = R"(
        hive TestHive2 {
            bowl test_field3: flot;
        }

        hive TestHive {
            bowl test_field1: flot = 100.0;
            bowl test_field2: flot = 300.0;
            bowl test_nested_hive: TestHive2;
        }

        blok bonk_main {
            bowl result = @TestHive[test_field1 = 1.0, test_nested_hive = @TestHive2[test_field3 = 3.0]];

            test_field3 of test_nested_hive of result = 4.0;

            bonk result;
        }
    )";

    const char* c_source = R"(
        #include <stdio.h>

        struct TestHive2 {
            float test_field3;
        };

        struct TestHive {
            float test_field1;
            float test_field2;
            struct TestHive2* test_nested_hive;
        };

        struct TestHive* bonk_main();

        int main() {
            struct TestHive* bonk_response = bonk_main();
            printf("%f %f %f",
                   bonk_response->test_field1,
                   bonk_response->test_field2,
                   bonk_response->test_nested_hive->test_field3);
        }
    )";

    ASSERT_TRUE(run_bonk_with_counterpart(bonk_source, c_source, "test"));
    EXPECT_EQ(run_executable("test"), "1.000000 300.000000 4.000000");
}

TEST(TestQBEFullCycle, TestFibonacci) {
    const char* bonk_source = R"(
        blok fibonacci[bowl input: flot] {
            bowl result: flot;

            input < 2.0 and { bonk input; };
            bonk @fibonacci[input = input - 1.0] + @fibonacci[input = input - 2.0];
        }
    )";

    const char* c_source = R"(
        #include <stdio.h>

        float fibonacci(float input);

        int main() {
            printf("%f %f %f", fibonacci(1.0), fibonacci(5.0), fibonacci(10.0));
        }
    )";

    ASSERT_TRUE(run_bonk_with_counterpart(bonk_source, c_source, "test"));
    EXPECT_EQ(run_executable("test"), "1.000000 5.000000 55.000000");
}