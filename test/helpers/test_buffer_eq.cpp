
#include <gtest/gtest.h>
#include "../helpers/test_buffer_eq.hpp"

static const char HEX_DIGITS[] = "0123456789abcdef";

void test_buffer_eq(const std::string& hex_buffer, const std::vector<char>& real_buffer) {
    std::string real_hex_buffer;

    for (int i = 0; i < real_buffer.size(); i++) {
        unsigned char byte = real_buffer[i];

        real_hex_buffer += HEX_DIGITS[byte / 16];
        real_hex_buffer += HEX_DIGITS[byte % 16];
    }

    ASSERT_EQ(hex_buffer, real_hex_buffer);
}
