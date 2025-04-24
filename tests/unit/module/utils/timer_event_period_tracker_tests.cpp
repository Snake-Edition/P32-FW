#include <catch2/catch.hpp>
#include <utils/timing/timer_event_period_tracker.hpp>

TEST_CASE("timer_event_period_tracker", "[timer_event_period_tracker]") {
    TimerEventPeriodTracker t;

    // No data yet - we should get an invalid result
    REQUIRE(t.period_unsafe() == TimerEventPeriodTracker::invalid_period);

    // Overflow shouldn't do anything
    t.handle_timer_overflow();
    REQUIRE(t.period_unsafe() == TimerEventPeriodTracker::invalid_period);

    // Still no data
    t.handle_event(10);
    REQUIRE(t.period_unsafe() == TimerEventPeriodTracker::invalid_period);

    // Now we should get first data
    t.handle_event(20);
    REQUIRE(t.period_unsafe() == 10);

    t.handle_event(21);
    REQUIRE(t.period_unsafe() == 1);

    t.handle_timer_overflow();
    t.handle_event(1);
    REQUIRE(t.period_unsafe() == 65536 + 1 - 21);

    t.handle_timer_overflow();
    t.handle_event(1);
    REQUIRE(t.period_unsafe() == 65536);

    t.handle_timer_overflow();
    t.handle_event(12500);
    REQUIRE(t.period_unsafe() == 65536 + 12500 - 1);

    t.handle_timer_overflow();
    t.handle_timer_overflow();
    t.handle_event(1);
    REQUIRE(t.period_unsafe() == 65536 * 2 + 1 - 12500);

    for (uint32_t i = 0; i < TimerEventPeriodTracker::max_timer_overflows; i++) {
        INFO(i);

        t.handle_event(1);

        for (uint32_t j = 0; j < i; j++) {
            t.handle_timer_overflow();
        }

        t.handle_event(1);
        REQUIRE(t.period_unsafe() == 65536 * i);
    }

    {
        t.handle_event(1);

        for (uint32_t i = 0; i < TimerEventPeriodTracker::max_timer_overflows; i++) {
            t.handle_timer_overflow();
        }

        UNSCOPED_INFO("The last event was too long ago -> invalid period_unsafe");
        REQUIRE(t.period_unsafe() == TimerEventPeriodTracker::invalid_period);

        t.handle_event(1);
        UNSCOPED_INFO("The distance between two events was too much -> still invalid period_unsafe");
        REQUIRE(t.period_unsafe() == TimerEventPeriodTracker::invalid_period);

        t.handle_event(2);
        REQUIRE(t.period_unsafe() == 1);
    }
}
