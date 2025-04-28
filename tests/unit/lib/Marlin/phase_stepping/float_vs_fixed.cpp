#include "lut_old.hpp"
#include <lut.hpp>

#include <catch2/catch.hpp>

using namespace phase_stepping;

namespace {

void fill_test_data(MotorPhaseCorrection &dest) {
    // Values measured on an actual printer, in 6.2 firmware for the Y-Back.
    dest[2].mag = 0.03097;
    dest[2].pha = 3.06143;
    dest[4].mag = 0.01513;
    dest[4].pha = 0.78695;
}

void uncalibrated(MotorPhaseCorrection &) {
    // Intentionally left blank.
}

typedef void (*fill_data)(MotorPhaseCorrection &);

void similar_values(int a, int b, const char *name) {
    INFO("Val " << name << " " << a << " " << b);
    // Check a and b differ by at most 5%
    const double diff = std::abs(a - b);
    if (diff <= 1) {
        // Off by one can happen at any time, unfortunately.
        return;
    }
    // The smaller one of them, but at least 1 to make division work.
    const double divisor = std::max(1, std::min(std::abs(a), std::abs(b)));
    // Allow up to 5% difference for rounding errors.
    REQUIRE(diff / divisor < 0.05);
}

} // namespace

// We want the "old" float and the "new" fixed implementation to return the
// same values (+- some rounding errors).
TEST_CASE("PhaseStepping - fixed vs float") {
    fill_data fill;
    const char *name;
    SECTION("Uncalibrated") {
        fill = uncalibrated;
        name = "Uncalibrated";
    }

    SECTION("Real printer") {
        fill = fill_test_data;
        name = "Real printer";
    }
    CorrectedCurrentLut table;
    table.modify_correction_table(fill);

    CorrectedCurrentLutSimple simple;
    simple.modify_correction_table(fill);

    phase_stepping_old::CorrectedCurrentLut old_table;
    old_table.modify_correction(fill);

    for (int i = 0; i < opts::MOTOR_PERIOD; i++) {
        INFO("Check " << name << ":" << i);
        const auto correction_real = table.get_current(i);
        const auto correction_old = old_table.get_current(i);
        const auto correction_simple = simple.get_current(i);

        // Preliminary check that the original was always OK.
        similar_values(correction_simple.a, correction_old.a, "Old-simple a");
        similar_values(correction_simple.b, correction_old.b, "Old-simple b");

        // Now check the new implementation.
        similar_values(correction_real.a, correction_simple.a, "Simple a");
        similar_values(correction_real.b, correction_simple.b, "Simple b");

        similar_values(correction_real.a, correction_old.a, "Old a");
        similar_values(correction_real.b, correction_old.b, "Old b");
    }
}
