#include <catch2/catch.hpp>
#include <round_fixed.hpp>

using namespace phase_stepping;

// Rounding to nearest fixed-point number, by definition / reusing floats to do it.
template <class T>
T round_fixed_floaty(T val, int bits) {
    int offset = 1 << bits;
    long double real_value = static_cast<long double>(val) / static_cast<long double>(offset);
    T result = roundl(real_value);
    return static_cast<T>(result);
}

TEST_CASE("Round fixed") {
    SECTION("Somewhat rounding") {
        for (size_t i = 0; i < 1000; i++) {
            long val = random() - std::numeric_limits<long>::max() / 2;
            int bits = random() % 16;

            long expected = round_fixed_floaty(val, bits);

            INFO("val: " << val << ", bits: " << bits);
            CHECK(round_fixed(val, bits) == expected);
        }
    }

    SECTION("Specific") {
        CHECK(round_fixed(8, 1) == round_fixed_floaty(8, 1));
        CHECK(round_fixed(9, 1) == round_fixed_floaty(9, 1));
        CHECK(round_fixed(7, 1) == round_fixed_floaty(7, 1));
    }
}
