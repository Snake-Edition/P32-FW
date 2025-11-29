/// str_utils tests

#include <string.h>
#include <iostream>
#include <cstdint>
#include <vector>

// #define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one cpp file
#include "catch2/catch.hpp"

using Catch::Matchers::Equals;

#include <lang/string_view_utf8.hpp>
#include <str_utils.hpp>

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
