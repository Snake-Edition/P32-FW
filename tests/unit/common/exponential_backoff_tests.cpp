
#include <utils/exponential_backoff.hpp>

#include <catch2/catch.hpp>

TEST_CASE("Exponential backoff") {
    buddy::ExponentialBackoff<uint32_t, 3, 17> backoff;
    REQUIRE_FALSE(backoff.get().has_value());
    backoff.fail();
    REQUIRE(backoff.get() == 3);
    backoff.fail();
    REQUIRE(backoff.get() == 6);
    backoff.fail();
    REQUIRE(backoff.get() == 12);
    backoff.fail();
    REQUIRE(backoff.get() == 17);
    backoff.fail();
    REQUIRE(backoff.get() == 17);
    backoff.reset();
    REQUIRE_FALSE(backoff.get().has_value());
}
