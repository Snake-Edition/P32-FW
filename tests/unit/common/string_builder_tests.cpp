#include <catch2/catch.hpp>

#include <utils/string_builder.hpp>

using Catch::Matchers::Equals;

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
                CHECK(b.str_nocheck()[b.char_count()] == '\0');
            }

            {
                auto ptr = b.alloc_chars(1); // Allocing something after problem should still not work
                CHECK(!ptr);
            }

        } while (std::next_permutation(overfill_order.begin(), overfill_order.end()));
    }

    SECTION("truncate multi-byte character") {

        ArrayStringBuilder<9> a;
        a.append_string("ププ");
        CHECK(a.is_ok());
        CHECK_THAT(a.str_nocheck(), Equals("ププ"));

        a.append_string("プ");
        CHECK(a.is_problem());
        CHECK_THAT(a.str_nocheck(), Equals("ププ"));

        ArrayStringBuilder<4> aa;
        aa.append_string("ププ");
        CHECK(aa.is_problem());
        CHECK_THAT(aa.str_nocheck(), Equals("プ"));

        ArrayStringBuilder<9> b;
        b.append_std_string_view("ププ");
        CHECK(b.is_ok());
        CHECK_THAT(b.str_nocheck(), Equals("ププ"));

        b.append_std_string_view("プ");
        CHECK(b.is_problem());
        CHECK_THAT(b.str_nocheck(), Equals("ププ"));

        ArrayStringBuilder<13> c;
        c.append_printf("%s%s", "ププ", "ププ");
        CHECK(c.is_ok());
        CHECK_THAT(c.str_nocheck(), Equals("ププププ"));

        c.append_printf("%s", "プ");
        CHECK(c.is_problem());
        CHECK_THAT(c.str_nocheck(), Equals("ププププ"));

        auto str = string_view_utf8::MakeCPUFLASH("ププ");
        ArrayStringBuilder<12> d;
        d.append_string_view(str);
        CHECK(d.is_ok());
        CHECK_THAT(d.str_nocheck(), Equals("ププ"));

        d.append_string_view(str);
        CHECK(d.is_problem());
        CHECK_THAT(d.str_nocheck(), Equals("プププ"));
    }

    SECTION("printf cropping") {
        ArrayStringBuilder<8> b;
        b.append_printf("123456%i", 56);
        CHECK(b.is_problem());
        CHECK_THAT(b.str_nocheck(), Equals("1234565"));
    }

    SECTION("append_std_string_view cropping") {
        ArrayStringBuilder<8> b;
        b.append_std_string_view("1234");
        CHECK(b.is_ok());
        CHECK_THAT(b.str_nocheck(), Equals("1234"));

        b.append_std_string_view("56789");
        CHECK(b.is_problem());
        CHECK_THAT(b.str_nocheck(), Equals("1234567"));
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
