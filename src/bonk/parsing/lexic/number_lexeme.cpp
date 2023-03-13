/*
 * Math number lexeme:
 * $NUMBER := '0x'DIGITS(16)'0o'$DIGITS(8)|'0b'$DIGITS(2)|$DECIMAL
 * $DECIMAL := ($DIGITS)(.($DIGITS))?(['E'|'e']($DIGITS))?
 * $DIGITS($RADIX) := [0-9|a-c]+
 */

#include "number_lexeme.hpp"
#include "../../compiler.hpp"

namespace bonk {

static unsigned char char_to_digit_radix(char c, int radix) {
    if (c < '0')
        return -1;

    if (radix <= 10) {
        if (c > '0' + (radix - 1))
            return -1;
    }

    if (c <= '9')
        return c - '0';

    if (radix <= 16) {
        c = tolower(c);
        if (c < 'a')
            return -1;
        if (c > 'a' + (radix - 11))
            return -1;
        return c - 'a' + 10;
    }

    return -1;
}

bool LexicalAnalyzer::parse_number_lexeme(Lexeme* target) {
    int fraction_digits = 0;
    long long integer_result = 0;
    double float_result = 0;
    double fraction_factor = 1;
    double fraction = 0;
    char c = '\0';
    int radix = 10;

    bool rather_double = false;

    int mantissa_digits = parse_digits_lexeme(10, &integer_result, &float_result);
    if (mantissa_digits == 0 && next_char() != '.') {
        linked_compiler.error().at(current_position) << "expected number";
        return false;
    }

    if (mantissa_digits == 1 && integer_result == 0) {
        c = next_char();
        if (c == 'x')
            radix = 16;
        else if (c == 'o')
            radix = 8;
        else if (c == 'b')
            radix = 2;

        if (radix != 10) {
            eat_char();
            mantissa_digits = parse_digits_lexeme(radix, &integer_result, &float_result);
            if (mantissa_digits == 0) {
                linked_compiler.error().at(current_position) << "expected number";
                return false;
            }
        }
    }

    c = next_char();
    if (radix == 10 && c == '.') {
        rather_double = true;
        eat_char();
        fraction_digits = parse_digits_lexeme(radix, nullptr, &fraction);
        if (fraction_digits == 0) {
            linked_compiler.error().at(current_position) << "expected fraction";
            return false;
        }
        for (int i = 0; i < fraction_digits; i++)
            fraction_factor *= 10;
        float_result += fraction / fraction_factor;
    }

    c = next_char();
    if (radix == 10 && (c == 'e' || c == 'E')) {
        rather_double = true;
        eat_char();
        c = next_char();
        bool sign = true;
        if (c == '+' || c == '-') {
            sign = c == '+';
            eat_char();
        }
        long long exponent = 0;
        int exponent_digits = parse_digits_lexeme(radix, &exponent, nullptr);
        if (exponent_digits == 0) {
            linked_compiler.error().at(current_position) << "exponent is empty";
            return false;
        }

        for (int i = 0; i < exponent; i++) {
            if (sign) {
                float_result *= 10;
                integer_result *= 10;

                if (fraction_digits > 0) {
                    fraction_factor /= 10;
                    integer_result += (long long)(fraction / fraction_factor) % 10;
                }
            } else {
                float_result /= 10;
                integer_result /= 10;
            }
        }
    }

    target->type = LexemeType::l_number;
    NumberLexeme number_lexeme;

    number_lexeme.contents.double_value = float_result;
    number_lexeme.contents.integer_value = integer_result;
    number_lexeme.contents.kind =
        rather_double ? NumberConstantKind::rather_double : NumberConstantKind::rather_integer;

    target->data = number_lexeme;

    return true;
}

int LexicalAnalyzer::parse_digits_lexeme(int radix, long long* integer_value, double* float_value) {
    long long integer_result = 0;
    double float_result = 0;
    int digits = 0;
    unsigned char digit = '\0';

    while (true) {
        digit = char_to_digit_radix(next_char(), radix);
        if (digit == (unsigned char)-1)
            break;

        eat_char();
        integer_result *= radix;
        float_result *= radix;
        integer_result += digit;
        float_result += digit;
        digits++;
    }

    if (integer_value)
        *integer_value = integer_result;
    if (float_value)
        *float_value = float_result;
    return digits;
}

} // namespace bonk