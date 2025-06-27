#include <catch2/catch.hpp>
#include <utils/timing/combo_counter.hpp>

TEST_CASE("ComboCounter::uint8_t") {
    ComboCounter<uint8_t, uint8_t> counter(20);
    CHECK(counter.current_combo() == 0);

    SECTION("Basic functionality") {
        counter.step(1, true);
        CHECK(counter.current_combo() == 1);

        counter.step(2, false);
        CHECK(counter.current_combo() == 1);

        counter.step(9, false);
        CHECK(counter.current_combo() == 1);

        counter.step(10, true);
        CHECK(counter.current_combo() == 2);

        counter.step(30, true);
        CHECK(counter.current_combo() == 3);

        counter.step(50, false);
        CHECK(counter.current_combo() == 3);

        counter.step(51, true);
        CHECK(counter.current_combo() == 1);

        counter.step(72, false);
        CHECK(counter.current_combo() == 0);
    }

    SECTION("Time wrapping") {
        counter.step(250, true);
        CHECK(counter.current_combo() == 1);

        counter.step(1, true);
        CHECK(counter.current_combo() == 2);

        counter.step(250, true);
        CHECK(counter.current_combo() == 1);

        counter.step(1, false);
        CHECK(counter.current_combo() == 1);

        counter.step(30, false);
        CHECK(counter.current_combo() == 0);
    }

    SECTION("Combo overflow") {
        for (int time = 0; time < 512; time++) {
            counter.step(uint8_t(time), true);
        }
        CHECK(counter.current_combo() == 255);
    }
}
