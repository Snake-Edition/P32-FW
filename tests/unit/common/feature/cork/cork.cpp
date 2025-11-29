#include <feature/cork/tracker.hpp>

#include <catch2/catch.hpp>

extern "C" uint32_t rand_u() {
    return 42;
}

TEST_CASE("Cork tracker") {
    buddy::cork::Tracker tracker;

    REQUIRE(tracker.clear_cnt() == 0);

    auto cork_1 = tracker.new_cork();
    REQUIRE(cork_1.has_value());

    auto cork_2 = tracker.new_cork();
    REQUIRE(cork_2.has_value());

    // Even though the random returns the same value, it doesn't collide.
    REQUIRE(cork_1->get_cookie() != cork_2->get_cookie());

    // Out of slots.
    REQUIRE(!tracker.new_cork().has_value());

    REQUIRE(!cork_1->is_done_and_consumed());

    // Non-existing cookie
    tracker.mark_done(1122);
    REQUIRE(!cork_1->is_done_and_consumed());
    REQUIRE(!cork_2->is_done_and_consumed());

    // One of them
    tracker.mark_done(cork_1->get_cookie());
    REQUIRE(cork_1->is_done_and_consumed());
    REQUIRE(!cork_2->is_done_and_consumed());

    // Now we can get a new one
    auto cork_3 = tracker.new_cork();
    REQUIRE(cork_3.has_value());

    REQUIRE(tracker.clear_cnt() == 0);
    tracker.clear();
    REQUIRE(tracker.clear_cnt() == 1);
    REQUIRE(cork_2->is_done_and_consumed());
    REQUIRE(cork_3->is_done_and_consumed());
}
