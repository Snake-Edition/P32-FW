/// str_utils tests

#include <string.h>
#include <iostream>
#include <cstdint>
#include <vector>

// #define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one cpp file
#include "catch2/catch.hpp"

using Catch::Matchers::Equals;

#include <str_utils.hpp>
#include "lang/string_view_utf8.hpp"

TEST_CASE("StringBuilder", "[strbuilder]") {
    SECTION("empty init") {
        ArrayStringBuilder<64> b;
        CHECK(b.is_ok());
        CHECK(b.char_count() == 0);
        CHECK_THAT(b.str_nocheck(), Equals(""));
    }

    SECTION("basic appends") {
        ArrayStringBuilder<64> b;

        b.append_string("test");
        CHECK(b.is_ok());
        CHECK_THAT(b.str_nocheck(), Equals("test"));

        b.append_string(" test2");
        CHECK(b.is_ok());
        CHECK_THAT(b.str_nocheck(), Equals("test test2"));

        b.append_char('X');
        CHECK(b.is_ok());
        CHECK_THAT(b.str_nocheck(), Equals("test test2X"));

        char *ptr = b.alloc_chars(2);
        CHECK(ptr);
        *ptr++ = 'Y';
        *ptr++ = 'Z';
        CHECK(b.is_ok());
        CHECK_THAT(b.str_nocheck(), Equals("test test2XYZ"));

        b.append_printf(" %s %i %g", "haha", 3, 5.0f);
        CHECK(b.is_ok());
        CHECK_THAT(b.str_nocheck(), Equals("test test2XYZ haha 3 5"));

        b.append_std_string_view("posl");
        CHECK(b.is_ok());
        CHECK_THAT(b.str_nocheck(), Equals("test test2XYZ haha 3 5posl"));
    }

    SECTION("exact fill") {
        ArrayStringBuilder<8> b;

        b.append_string("x7chars"); // Should exactly fit the buffer (incl. term \0)
        CHECK(b.is_ok());
        CHECK_THAT(b.str_nocheck(), Equals("x7chars"));

        b.append_string("whatever");
        CHECK(!b.is_ok());
        CHECK(b.is_problem());
        CHECK_THAT(b.str_nocheck(), Equals("x7chars"));
    }

    SECTION("overfill") {
        std::vector<int> overfill_order(3); // N of cmds for permutating
        std::iota(overfill_order.begin(), overfill_order.end(), 0);

        do {
            constexpr auto cap = 8;
            ArrayStringBuilder<cap> b;

            b.append_string("abc");
            CHECK(b.is_ok());
            CHECK_THAT(b.str_nocheck(), Equals("abc"));

            for (const int cmd : overfill_order) {
                switch (cmd) {

                case 0:
                    b.append_string("this does not fit");
                    break;

                case 1: {
                    auto ptr = b.alloc_chars(8);
                    CHECK(!ptr);
                    break;
                }

                case 2:
                    b.append_printf("s %s", "something really long");
                    break;
                }

                CHECK(b.is_problem());
                CHECK_THAT(b.str_nocheck(), Equals("abc"));
                CHECK(b.char_count() == 3);
                CHECK(b.str_nocheck()[b.char_count()] == '\0');
            }

            {
                auto ptr = b.alloc_chars(1); // Allocing something after problem should still not work
                CHECK(!ptr);
            }

        } while (std::next_permutation(overfill_order.begin(), overfill_order.end()));
    }

    SECTION("append_float") {
        const auto afl_check = [](double val, const char *expected, const StringBuilder::AppendFloatConfig &cfg = {}) {
            ArrayStringBuilder<16> b;
            b.append_float(val, cfg);
            CHECK_THAT(b.str_nocheck(), Equals(expected));
        };

        afl_check(0, "0");
        afl_check(0.3, "0.3");
        afl_check(0.29, "0.29");
        afl_check(-15.01, "-15.01");

        afl_check(0, "0", { .skip_zero_before_dot = true });
        afl_check(0.91, ".91", { .skip_zero_before_dot = true });
        afl_check(-0.313, "-0.313", { .skip_zero_before_dot = true });
        afl_check(3.13, "3.13", { .skip_zero_before_dot = true });

        afl_check(-0.1, "-0.100", { .max_decimal_places = 3, .all_decimal_places = true });
        afl_check(-0.1, "-0.10", { .max_decimal_places = 2, .all_decimal_places = true });
        afl_check(-0.001, "-0.001", { .max_decimal_places = 3, .all_decimal_places = true });

        afl_check(-0.001, "-0.001", { .max_decimal_places = 3 });
        afl_check(0.00099, "0.001", { .max_decimal_places = 3 });
        afl_check(-0.00099, "-0.001", { .max_decimal_places = 3 });
        afl_check(0.0005, "0.001", { .max_decimal_places = 3 });
        afl_check(0.00049, "0", { .max_decimal_places = 3 });
        afl_check(-0.0005, "-0.001", { .max_decimal_places = 3 });
        afl_check(-0.00049, "0", { .max_decimal_places = 3 });
    }
}

template <typename T>
void test_from_chars_light() {
    from_chars_light_result res;
    T val;

    SECTION("basic") {
        std::string str = "123";
        res = from_chars_light(str.c_str(), str.c_str() + str.size(), val, 10);
        CHECK(res.ec == std::errc {});
        CHECK(val == 123);
    }

    SECTION("basic with spaces") {
        std::string str = " 123 ";
        res = from_chars_light(str.c_str(), str.c_str() + str.size(), val, 10);
        CHECK(res.ec == std::errc {});
        CHECK(val == 123);
    }

    SECTION("hex") {
        std::string str = "5F";
        res = from_chars_light(str.c_str(), str.c_str() + str.size(), val, 16);
        CHECK(res.ec == std::errc {});
        CHECK(val == 0x5F);
    }

    SECTION("empty") {
        std::string str = "XX";
        res = from_chars_light(str.c_str(), str.c_str() + str.size(), val, 10);
        CHECK(res.ec == std::errc::invalid_argument);
    }

    SECTION("Text bounds") {
        std::string str = " 123456789 ";
        res = from_chars_light(str.c_str(), str.c_str() + 3, val, 10);
        CHECK(res.ec == std::errc {});
        CHECK(val == 12);
    }

    SECTION("Min bound") {
        int64_t min = std::numeric_limits<decltype(val)>::min();
        std::string str = std::to_string(min);
        res = from_chars_light(str.c_str(), str.c_str() + str.size(), val, 10);
        CHECK(res.ec == std::errc {});
        CHECK(val == min);
    }

    SECTION("max bound") {
        int64_t max = std::numeric_limits<decltype(val)>::max();
        std::string str = std::to_string(max);
        res = from_chars_light(str.c_str(), str.c_str() + str.size(), val, 10);
        CHECK(res.ec == std::errc {});
        CHECK(val == max);
    }

    if constexpr (std::is_signed<T>()) {
        SECTION("Min bound -1") {
            std::string str;
            int64_t min = static_cast<int64_t>(std::numeric_limits<decltype(val)>::min());
            str = std::to_string(min);
            str[std::size(str) - 1] = str[std::size(str) - 1] + 1; // one, but since this is negative number, actually add one to last digit
            res = from_chars_light(str.c_str(), str.c_str() + str.size(), val, 10);
            CHECK(res.ec == std::errc::result_out_of_range);
        }
    }
    SECTION("max bound +1") {
        uint64_t max = static_cast<int64_t>(std::numeric_limits<decltype(val)>::max());
        std::string str = std::to_string(max);
        str[std::size(str) - 1] = str[std::size(str) - 1] + 1; // add one from last digit
        res = from_chars_light(str.c_str(), str.c_str() + str.size(), val, 10);
        CHECK(res.ec == std::errc::result_out_of_range);
    }
}

TEST_CASE("from_chars_light:int8_t") {
    test_from_chars_light<int8_t>();
}

TEST_CASE("from_chars_light:int16_t") {
    test_from_chars_light<int16_t>();
}

TEST_CASE("from_chars_light:int32_t") {
    test_from_chars_light<int32_t>();
}

TEST_CASE("from_chars_light:int64_t") {
    test_from_chars_light<int64_t>();
}

TEST_CASE("from_chars_light:uint8_t") {
    test_from_chars_light<uint8_t>();
}

TEST_CASE("from_chars_light:uint16_t") {
    test_from_chars_light<uint16_t>();
}

TEST_CASE("from_chars_light:uint32_t") {
    test_from_chars_light<uint32_t>();
}

TEST_CASE("from_chars_light:uint64_t") {
    test_from_chars_light<uint64_t>();
}

TEST_CASE("from_chars_light:float") {
    float val;
    from_chars_light_result res;

    SECTION("basic") {
        std::string str = "123.123";
        res = from_chars_light(str.c_str(), str.c_str() + str.size(), val);
        CHECK(res.ec == std::errc {});
        CHECK(std::abs(val - 123.123f) < 0.0001);
    }

    SECTION("basic with spaces") {
        std::string str = " 123.123 ";
        res = from_chars_light(str.c_str(), str.c_str() + str.size(), val);
        CHECK(res.ec == std::errc {});
        CHECK(std::abs(val - 123.123f) < 0.0001);
    }

    SECTION("empty") {
        std::string str = "XX";
        res = from_chars_light(str.c_str(), str.c_str() + str.size(), val);
        CHECK(res.ec == std::errc::invalid_argument);
    }

    SECTION("Text bounds") {
        std::string str = " 123456789 ";
        res = from_chars_light(str.c_str(), str.c_str() + 3, val);
        CHECK(res.ec == std::errc {});
        CHECK(val == 12);
    }
}
