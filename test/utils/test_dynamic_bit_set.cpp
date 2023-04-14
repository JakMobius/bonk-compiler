
#include <gtest/gtest.h>
#include "utils/dynamic_bitset.hpp"

TEST(DynamicBitSet, SimpleTest) {
    bonk::DynamicBitSet bitset(10);

    ASSERT_EQ(bitset.size(), 10);
    ASSERT_FALSE(bitset.any());

    bitset.set();
    ASSERT_TRUE(bitset.any());

    auto&& element = bitset[3];

    for (auto i = 0; i < 10; i++) {
        ASSERT_TRUE(bitset[i]);
    }

    element = false;

    for (auto i = 0; i < 10; i++) {
        if (i == 3) {
            ASSERT_FALSE(bitset[i]);
        } else {
            ASSERT_TRUE(bitset[i]);
        }
    }
}

TEST(DynamicBitSet, TestAnd) {
    bonk::DynamicBitSet bitset1({1, 0, 1, 0, 1, 0, 1, 0});
    bonk::DynamicBitSet bitset2({1, 1, 0, 0, 1, 1, 0, 0});

    auto bitset3 = bitset1 & bitset2;

    ASSERT_EQ(bitset3.size(), 8);
    ASSERT_EQ(bitset3, bonk::DynamicBitSet({1, 0, 0, 0, 1, 0, 0, 0}));
    ASSERT_NE(bitset3, bonk::DynamicBitSet({0, 1, 0, 0, 1, 0, 0, 0}));
}

TEST(DynamicBitSet, TestOr) {
    bonk::DynamicBitSet bitset1({1, 0, 1, 0, 1, 0, 1, 0});
    bonk::DynamicBitSet bitset2({1, 1, 0, 0, 1, 1, 0, 0});

    auto bitset3 = bitset1 | bitset2;

    ASSERT_EQ(bitset3.size(), 8);
    ASSERT_EQ(bitset3, bonk::DynamicBitSet({1, 1, 1, 0, 1, 1, 1, 0}));
}

TEST(DynamicBitSet, TestXor) {
    bonk::DynamicBitSet bitset1({1, 0, 1, 0, 1, 0, 1, 0});
    bonk::DynamicBitSet bitset2({1, 1, 0, 0, 1, 1, 0, 0});

    auto bitset3 = bitset1 ^ bitset2;

    ASSERT_TRUE(bitset3.any());
    ASSERT_EQ(bitset3.size(), 8);
    ASSERT_EQ(bitset3, bonk::DynamicBitSet({0, 1, 1, 0, 0, 1, 1, 0}));
}

TEST(DynamicBitSet, TestSub) {
    bonk::DynamicBitSet bitset1({1, 0, 1, 0, 1, 0, 1, 0});
    bonk::DynamicBitSet bitset2({1, 1, 0, 0, 1, 1, 0, 0});

    auto bitset3 = bitset1 - bitset2;

    ASSERT_TRUE(bitset3.any());
    ASSERT_EQ(bitset3.size(), 8);
    ASSERT_EQ(bitset3, bonk::DynamicBitSet({0, 0, 1, 0, 0, 0, 1, 0}));
}

TEST(DynamicBitSet, SimpleLargeTest) {
    // Check that the bitset can handle more than its inline capacity

    bonk::DynamicBitSet bitset(511);

    ASSERT_EQ(bitset.size(), 511);
    ASSERT_FALSE(bitset.any());

    bitset.set();
    ASSERT_TRUE(bitset.any());

    auto&& element = bitset[3];

    for (auto i = 0; i < 10; i++) {
        ASSERT_TRUE(bitset[i]);
    }

    element = false;

    for (auto i = 0; i < 511; i++) {
        if (i == 3) {
            ASSERT_FALSE(bitset[i]);
        } else {
            ASSERT_TRUE(bitset[i]);
        }
    }
}