#include <catch2/catch.hpp>
#include <utils/timing/timer.hpp>

using namespace utils;

TEST_CASE("Timer::uint8_t", "[timer]") {
    // First of all, check that math still works
    CHECK(1 + 1 == 2);

    Timer<uint8_t> timer(50);
    using State = decltype(timer)::State;

    {
        CHECK(timer.state() == State::idle);

        // Not running - elapsed should always return 0
        CHECK(timer.elapsed(2) == 0);
        CHECK(timer.elapsed(255) == 0);
    }

    {
        timer.restart(1);
        CHECK(timer.state() == State::running);

        CHECK(timer.elapsed(1) == 0);
        CHECK(timer.elapsed(2) == 1);
        CHECK(timer.elapsed(255) == 254);
        CHECK(timer.elapsed(0) == 255);

        CHECK(!timer.check(1));
        CHECK(timer.is_running());

        CHECK(!timer.check(50));
        CHECK(timer.is_running());

        CHECK(timer.check(51));
        CHECK(timer.state() == State::finished);

        CHECK(!timer.check(51));

        timer.stop();
        CHECK(timer.state() == State::idle);
    }

    {
        timer.restart(220);
        CHECK(timer.is_running());

        CHECK(timer.elapsed(220) == 0);
        CHECK(timer.elapsed(255) == 35);
        CHECK(timer.elapsed(14) == 50);
        CHECK(timer.elapsed(15) == 51);

        CHECK(!timer.check(13));
        CHECK(timer.is_running());

        CHECK(timer.check(14));
        CHECK(timer.state() == State::finished);
    }

    {
        timer.restart(220);
        CHECK(timer.is_running());

        CHECK(timer.check(20));
        CHECK(timer.state() == State::finished);
    }

    {
        timer.restart(220);
        CHECK(timer.is_running());

        timer.stop();
        CHECK(timer.state() == State::idle);
    }

    // Check that math still works after doing our tests
    // One can never be too sure
    CHECK(2 * 2 == 4);
}
