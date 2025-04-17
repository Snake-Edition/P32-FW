/*
 * CoreXY precise homing refinement - implementation
 * TODO: @wavexx: Add some documentation
 */

#include "homing_corexy.hpp"

// sanity checks
#ifdef PRECISE_HOMING
    #error "PRECISE_HOMING_COREXY is mutually exclusive with PRECISE_HOMING"
#endif
#ifdef HAS_TMC_WAVETABLE
    // Wavetable restoration needs to happen after refinement succeeds, and
    // not per-axis as currently done. Ensure the setting is not enabled by mistake.
    #error "PRECISE_HOMING_COREXY is not compatible with HAS_TMC_WAVETABLE"
#endif

#include "../planner.h"
#include "../stepper.h"
#include "../endstops.h"

#if ENABLED(CRASH_RECOVERY)
    #include "feature/prusa/crash_recovery.hpp"
#endif

#include <bsod.h>
#include <scope_guard.hpp>
#include <feature/phase_stepping/phase_stepping.hpp>
#include <feature/input_shaper/input_shaper_config.hpp>
#include <config_store/store_instance.hpp>
#include <metric.h>

#if HAS_TRINAMIC && defined(XY_HOMING_MEASURE_SENS_MIN)
    #include <configuration.hpp>
#endif

#pragma GCC diagnostic warning "-Wdouble-promotion"

namespace internal {
namespace {
    bool home_unstable = false; ///< Last homing stability state
    uint8_t probe_id = 0; ///< Probe id for metric cross-referencing
    uint8_t refine_id = 0; ///< Refine count for metric cross-referencing
    uint8_t cal_id = 0; ///< Calibration count for metric cross-referencing
} // namespace
} // namespace internal

METRIC_DEF(metric_phxy_meas, "phxy_meas", METRIC_VALUE_CUSTOM, 0, METRIC_ENABLED);
METRIC_DEF(metric_phxy_probe, "phxy_probe", METRIC_VALUE_CUSTOM, 0, METRIC_ENABLED);
METRIC_DEF(metric_phxy_sens, "phxy_sens", METRIC_VALUE_CUSTOM, 0, METRIC_ENABLED);
METRIC_DEF(metric_phxy_home, "phxy_home", METRIC_VALUE_CUSTOM, 0, METRIC_ENABLED);
METRIC_DEF(metric_phxy_orig, "phxy_orig", METRIC_VALUE_CUSTOM, 0, METRIC_ENABLED);

// convert raw AB steps to XY mm
void corexy_ab_to_xy(const xy_long_t &steps, xy_pos_t &mm) {
    const float x = static_cast<float>(steps.a + steps.b) / 2.f;
    const float y = static_cast<float>(CORESIGN(steps.a - steps.b)) / 2.f;
    mm.x = x * planner.mm_per_step[X_AXIS];
    mm.y = y * planner.mm_per_step[Y_AXIS];
}

// convert raw AB steps to XY mm and position in mini-steps
static void corexy_ab_to_xy(const xy_long_t &steps, xy_pos_t &mm, xy_long_t &pos_msteps) {
    const float x = static_cast<float>(steps.a + steps.b) / 2.f;
    const float y = static_cast<float>(CORESIGN(steps.a - steps.b)) / 2.f;
    mm.x = x * planner.mm_per_step[X_AXIS];
    mm.y = y * planner.mm_per_step[Y_AXIS];
    pos_msteps.x = LROUND(x * PLANNER_STEPS_MULTIPLIER);
    pos_msteps.y = LROUND(y * PLANNER_STEPS_MULTIPLIER);
}

// convert raw AB steps to XY mm, filling others from current state
void corexy_ab_to_xyze(const xy_long_t &steps, xyze_pos_t &mm) {
    corexy_ab_to_xy(steps, mm);
    LOOP_S_L_N(i, C_AXIS, XYZE_N) {
        mm[i] = planner.get_axis_position_mm((AxisEnum)i);
    }
}

// convert raw AB steps to XY mm and position in mini-steps, filling others from current state
static void corexy_ab_to_xyze(const xy_long_t &steps, xyze_pos_t &mm, xyze_long_t &pos_msteps) {
    pos_msteps = planner.get_position_msteps();
    corexy_ab_to_xy(steps, mm, pos_msteps);
    LOOP_S_L_N(i, C_AXIS, XYZE_N) {
        mm[i] = planner.get_axis_position_mm((AxisEnum)i);
    }
}

static void plan_raw_move(const xyze_pos_t target_mm, const xyze_long_t target_pos, const feedRate_t fr_mm_s) {
    planner._buffer_msteps_raw(target_pos, target_mm, fr_mm_s, active_extruder);
    planner.synchronize();
}

static void plan_corexy_raw_move(const xy_long_t &target_steps_ab, const feedRate_t fr_mm_s) {
    // reconstruct full final position
    xyze_pos_t target_mm;
    xyze_long_t target_pos_msteps;
    corexy_ab_to_xyze(target_steps_ab, target_mm, target_pos_msteps);

    plan_raw_move(target_mm, target_pos_msteps, fr_mm_s);
}

// TMC µsteps(phase) per Marlin µsteps
static constexpr int16_t phase_per_ustep(const AxisEnum axis) {
    // Originally, we read the microstep configuration from the driver; this no
    // longer make sense with 256 microsteps.
    // Thus, we use the printer defaults instead of stepper_axis(axis).microsteps();
    assert(axis <= AxisEnum::Z_AXIS);
    static const int MICROSTEPS[] = { X_MICROSTEPS, Y_MICROSTEPS, Z_MICROSTEPS };
    return 256 / MICROSTEPS[axis];
};

// TMC full cycle µsteps per Marlin µsteps
static constexpr int16_t phase_cycle_steps(const AxisEnum axis) {
    return 1024 / phase_per_ustep(axis);
}

static int16_t axis_mscnt(const AxisEnum axis) {
#if HAS_PHASE_STEPPING()
    return phase_stepping::logical_ustep(axis);
#else
    return stepper_axis(axis).MSCNT();
#endif
}

static int16_t phase_backoff_steps(const AxisEnum axis) {
    int16_t effectorBackoutDir; // Direction in which the effector mm coordinates move away from endstop.
    int16_t stepperCountDir; // Direction in which the TMC µstep count(phase) increases.
    switch (axis) {
    case X_AXIS:
        effectorBackoutDir = -X_HOME_DIR;
        stepperCountDir = INVERT_X_DIR ? -1 : 1;
        break;
    case Y_AXIS:
        effectorBackoutDir = -Y_HOME_DIR;
        stepperCountDir = INVERT_Y_DIR ? -1 : 1;
        break;
    default:
        bsod("invalid backoff axis");
    }

    const int16_t phaseCurrent = axis_mscnt(axis); // The TMC µsteps(phase) count of the current position
    const int16_t phaseDelta = ((stepperCountDir < 0) == (effectorBackoutDir < 0) ? phaseCurrent : 1024 - phaseCurrent);
    const int16_t phasePerStep = phase_per_ustep(axis);
    return int16_t((phaseDelta + phasePerStep / 2) / phasePerStep) * effectorBackoutDir;
}

static bool phase_aligned(AxisEnum axis) {
    const int16_t phase_cur = axis_mscnt(axis);
    const int16_t ustep_max = phase_per_ustep(axis) / 2;
    return (phase_cur <= ustep_max || phase_cur >= (1024 - ustep_max));
}

// Helper class to adjust machine settings for AB measurements using measure_axis_distance().
// Only the non-measured axis stepper is adjusted: the measured stepper is setup within
// measure_axis_distance() itself.
class SetupForMeasurement {
    static unsigned setup; // helper to ensure machine settings are set exactly once

    // "other" stepper original settings
    decltype(stepperX) &other_stepper;
    int32_t other_orig_cur;
    float other_orig_hold;

    // original IS settings
    std::optional<input_shaper::AxisConfig> is_config_orig[2];

public:
    [[nodiscard]] SetupForMeasurement(AxisEnum other_axis)
        : other_stepper(stepper_axis(other_axis)) {
        ++setup;
        assert(setup == 1);

        other_orig_cur = other_stepper.rms_current();
        other_orig_hold = other_stepper.hold_multiplier();
#ifdef XY_HOMING_HOLDING_CURRENT
        other_stepper.rms_current(XY_HOMING_HOLDING_CURRENT, 1.f);
#endif

        is_config_orig[A_AXIS] = input_shaper::get_axis_config(A_AXIS);
        is_config_orig[B_AXIS] = input_shaper::get_axis_config(B_AXIS);
        input_shaper::set_axis_config(A_AXIS, std::nullopt);
        input_shaper::set_axis_config(B_AXIS, std::nullopt);
    }

    ~SetupForMeasurement() {
        --setup;
        other_stepper.rms_current(other_orig_cur, other_orig_hold);
        input_shaper::set_axis_config(A_AXIS, is_config_orig[A_AXIS]);
        input_shaper::set_axis_config(B_AXIS, is_config_orig[B_AXIS]);
    }

    static bool is_setup() {
        return setup > 0;
    }
};

unsigned SetupForMeasurement::setup = 0;

// Axis measurement settings
struct measure_axis_params {
    float feedrate;
    uint16_t current;
#if HAS_TRINAMIC
    int8_t sensitivity;
#endif
};

/**
 * @brief Measure axis distance precisely
 * @param axis Axis to measure
 * @param origin_steps Initial stepper position
 * @param dist Maximum distance/direction to travel to hit an endstop
 * @param m_steps Measured steps
 * @param m_dist Measured distance
 * @param params Measured axis/stepper parameters
 * @return True on success
 */
static bool measure_axis_distance(AxisEnum axis, xy_long_t origin_steps, int32_t dist, int32_t &m_steps, float &m_dist, const measure_axis_params &params) {
    // full initial position
    const xyze_long_t initial_steps = { origin_steps.a, origin_steps.b, stepper.position(C_AXIS), stepper.position(E_AXIS) };
    xyze_pos_t initial_mm;
    corexy_ab_to_xyze(initial_steps, initial_mm);

    // full target position
    xyze_long_t target_steps = initial_steps;
    target_steps[axis] += dist;

    xyze_pos_t target_mm;
    xyze_long_t target_pos_msteps;
    corexy_ab_to_xy(target_steps, target_mm, target_pos_msteps);
    LOOP_S_L_N(i, C_AXIS, XYZE_N) {
        target_mm[i] = initial_mm[i];
    }
    const xyze_long_t initial_pos_msteps = planner.get_position_msteps();
    LOOP_S_L_N(i, C_AXIS, XYZE_N) {
        target_pos_msteps[i] = initial_pos_msteps[i];
    }

    // prepare stepper for the move
    assert(SetupForMeasurement::is_setup());
    const sensorless_t stealth_states = start_sensorless_homing_per_axis(axis);
    auto &axis_stepper = stepper_axis(axis);
    const int32_t axis_orig_cur = axis_stepper.rms_current();
    const float axis_orig_hold = axis_stepper.hold_multiplier();
    axis_stepper.rms_current(params.current, 1.f);
#if HAS_TRINAMIC
    const int8_t axis_orig_sens = axis_stepper.sgt();
    axis_stepper.sgt(params.sensitivity);
#endif
    ScopeGuard state_restorer([&]() {
#if HAS_TRINAMIC
        axis_stepper.sgt(axis_orig_sens);
#endif
        axis_stepper.rms_current(axis_orig_cur, axis_orig_hold);
    });

    // move towards the endstop
    endstops.enable(true);
    plan_raw_move(target_mm, target_pos_msteps, params.feedrate);
    const uint8_t hit = endstops.trigger_state();
    endstops.not_homing();

    xyze_long_t hit_steps;
    xyze_pos_t hit_mm;
    if (hit) {
        // resync position from steppers to get hit position
        endstops.hit_on_purpose();
        planner.reset_position();
        hit_steps = { stepper.position(A_AXIS), stepper.position(B_AXIS), stepper.position(C_AXIS), stepper.position(E_AXIS) };
        corexy_ab_to_xyze(hit_steps, hit_mm);
    } else {
        hit_steps = target_steps;
        hit_mm = target_mm;
    }
    end_sensorless_homing_per_axis(axis, stealth_states);

    // move back to starting point
    plan_raw_move(initial_mm, initial_pos_msteps, homing_feedrate(axis));
    if (planner.draining()) {
        return false;
    }

    // sanity checks
    const AxisEnum fixed_axis = (axis == B_AXIS ? A_AXIS : B_AXIS);
    if (hit_steps[fixed_axis] != initial_steps[fixed_axis] || initial_steps[fixed_axis] != stepper.position(fixed_axis)) {
        bsod("fixed axis moved unexpectedly");
    }
    if (initial_steps[axis] != stepper.position(axis)) {
        bsod("measured axis didn't return");
    }

    // result values
    m_steps = hit_steps[axis] - initial_steps[axis];
    m_dist = hypotf(hit_mm[X_AXIS] - initial_mm[X_AXIS], hit_mm[Y_AXIS] - initial_mm[Y_AXIS]);

    metric_record_custom(&metric_phxy_meas, ",a=%u,dir=%i p=%u,s=%li,d=%.3f",
        axis, dist >= 0 ? 1 : -1, internal::probe_id, (long)m_steps, (double)m_dist);

    return hit;
}

// Return measure axis default parameters
static measure_axis_params measure_axis_defaults(const AxisEnum axis) {
    measure_axis_params params;

#if HAS_TRINAMIC
    #ifdef XY_HOMING_MEASURE_SENS
    params.sensitivity = XY_HOMING_MEASURE_SENS;
    #else
    params.sensitivity = (axis == A_AXIS ? X_STALL_SENSITIVITY : Y_STALL_SENSITIVITY);
    #endif
#endif
#ifdef XY_HOMING_MEASURE_FR
    params.feedrate = XY_HOMING_MEASURE_FR;
#else
    params.feedrate = homing_feedrate(axis);
#endif
#ifdef XY_HOMING_MEASURE_CURRENT
    params.current = XY_HOMING_MEASURE_CURRENT;
#else
    params.current = (axis == A_AXIS ? X_CURRENT_HOME : Y_CURRENT_HOME);
#endif

    return params;
}

// Call measure_axis_distance() with calibrated parameters
// (or defaults in printers without measurement calibration)
static bool measure_axis_distance(AxisEnum axis, xy_long_t origin_steps, int32_t dist, int32_t &m_steps, float &m_dist) {
    measure_axis_params params;

#if HAS_TRINAMIC && defined(XY_HOMING_MEASURE_SENS_MIN)
    // get paramers from calibration
    const CoreXYHomeTMCSens calibrated_sens = config_store().corexy_home_tmc_sens.get();
    if (calibrated_sens.uninitialized()) {
        bsod("axis measurement without calibration");
    }

    params.sensitivity = calibrated_sens.sensitivity;
    params.feedrate = calibrated_sens.feedrate;
    params.current = calibrated_sens.current;
#else
    // static defaults
    params = measure_axis_defaults(axis);
#endif

    return measure_axis_distance(axis, origin_steps, dist, m_steps, m_dist, params);
}

/**
 * @brief Sum "axis" along the XY*val<> sequence "seq"
 * @tparam T Sequence type (normally deducted)
 * @tparam S sum type (normally deducted)
 * @param seq Sequence of XY*val
 * @param size Size of the sequence
 * @param axis Axis to sum
 * @return Resulting sum
 */
template <typename T, typename S = decltype(T::x)>
S sum_along(const T *seq, const size_t size, const size_t axis) {
    S sum = 0;
    for (size_t i = 0; i != size; ++i) {
        sum += seq[i][axis];
    }
    return sum;
}

/**
 * @brief Part of precise homing.
 * @param axis Physical axis to measure
 * @param c_dist AB cycle distance from the endstop
 * @param m_dist 1/2 distance from the endstop (mm)
 * @return True on success
 */
static bool measure_phase_cycles(AxisEnum axis, xy_pos_t &c_dist, xy_pos_t &m_dist) {
    // prepare for repeated measurements
    const AxisEnum other_axis = (axis == B_AXIS ? A_AXIS : B_AXIS);
    SetupForMeasurement setup_guard(other_axis);
    ++internal::probe_id;

    const int32_t measure_max_dist = (XY_HOMING_ORIGIN_OFFSET * 4) / planner.mm_per_step[axis];
    const int32_t measure_dir = (axis == B_AXIS ? -X_HOME_DIR : -Y_HOME_DIR);
    const xy_long_t origin_steps = { stepper.position(A_AXIS), stepper.position(B_AXIS) };
    constexpr int probe_n = 2;
    xy_long_t p_steps[probe_n];
    xy_pos_t p_dist[probe_n] = { -XY_HOMING_ORIGIN_BUMP_MAX_ERR, -XY_HOMING_ORIGIN_BUMP_MAX_ERR };

    uint8_t retry;
    for (retry = 0; retry != XY_HOMING_ORIGIN_BUMP_RETRIES; ++retry) {
        const uint8_t slot0 = retry % probe_n;
        const uint8_t slot1 = (retry + 1) % probe_n;

        // measure distance B+/B-
        if (!measure_axis_distance(axis, origin_steps, measure_max_dist * measure_dir, p_steps[slot1][1], p_dist[slot1][1])
            || !measure_axis_distance(axis, origin_steps, measure_max_dist * -measure_dir, p_steps[slot1][0], p_dist[slot1][0])) {
            if (!planner.draining()) {
                ui.status_printf_P(0, "Endstop not reached");
            }
            return false;
        }

        // keep signs positive
        LOOP_XY(i) {
            p_steps[slot1][i] = abs(p_steps[slot1][i]);
            p_dist[slot1][i] = abs(p_dist[slot1][i]);
        }

        const float p_diff[2] = {
            abs(p_dist[slot0][0] - p_dist[slot1][0]),
            abs(p_dist[slot0][1] - p_dist[slot1][1])
        };

        metric_record_custom(&metric_phxy_probe, ",a=%u p=%u,r=%u,d0=%.3f,d1=%.3f",
            axis, internal::probe_id, retry, (double)p_diff[0], (double)p_diff[1]);

        if (p_diff[0] < float(XY_HOMING_ORIGIN_BUMP_MAX_ERR) && p_diff[1] < float(XY_HOMING_ORIGIN_BUMP_MAX_ERR)) {
            break;
        }
    }
    if (retry == XY_HOMING_ORIGIN_BUMP_RETRIES) {
        ui.status_printf_P(0, "Axis measurement failed");
        return false;
    }

    // calculate the absolute cycle coordinates
    const float d1 = sum_along(p_steps, probe_n, 0) / float(probe_n);
    const float d2 = sum_along(p_steps, probe_n, 1) / float(probe_n);
    const float d = d1 + d2;
    const float a = d / 2.f;
    const float b = d1 - a;

    c_dist[0] = a / float(phase_cycle_steps(other_axis));
    c_dist[1] = b / float(phase_cycle_steps(axis));

    m_dist[0] = sum_along(p_dist, probe_n, 0) / float(probe_n);
    m_dist[1] = sum_along(p_dist, probe_n, 1) / float(probe_n);

    if (DEBUGGING(LEVELING)) {
        // measured distance and cycle
        SERIAL_ECHOLNPAIR("home ", physical_axis_codes[axis], "+ steps 0:", p_steps[0][1], " 1:", p_steps[1][1],
            " cycle A:", c_dist[0], " mm:", m_dist[1]);
        SERIAL_ECHOLNPAIR("home ", physical_axis_codes[axis], "- steps 0:", p_steps[0][0], " 1:", p_steps[1][0],
            " cycle B:", c_dist[1], " mm:", m_dist[0]);
    }
    return true;
}

// return true if the point is too close to the phase grid halfway point
static bool point_is_unstable(const xy_pos_t &c_dist, const xy_pos_t &origin) {
    static constexpr float threshold = 1. / 4;
    LOOP_XY(axis) {
        if (abs(fmod(c_dist[axis] - origin[axis], 1.f) - 0.5f) < threshold) {
            return true;
        }
    }
    return false;
}

// translate fractional cycle distance by origin and round to final AB grid
static xy_long_t cdist_translate(const xy_pos_t &c_dist, const xy_pos_t &origin) {
    xy_long_t c_ab;
    LOOP_XY(axis) {
        long o_int = long(roundf(origin[axis]));
        c_ab[axis] = long(roundf(c_dist[axis] - origin[axis])) + o_int;
    }
    return c_ab;
}

/**
 * @brief plan a relative move by full AB cycles around origin_steps
 * @param ab_off full AB cycles away from homing corner
 * @return new step position
 */
static xy_long_t plan_corexy_abgrid_move(const xy_long_t &origin_steps, const xy_long_t &ab_off, const float fr_mm_s) {
    const long a = ab_off[X_HOME_DIR == Y_HOME_DIR ? A_AXIS : B_AXIS] * -Y_HOME_DIR;
    const long b = ab_off[X_HOME_DIR == Y_HOME_DIR ? B_AXIS : A_AXIS] * -X_HOME_DIR;

    xy_long_t point_steps = {
        origin_steps[A_AXIS] + phase_cycle_steps(A_AXIS) * a,
        origin_steps[B_AXIS] + phase_cycle_steps(B_AXIS) * b
    };

    plan_corexy_raw_move(point_steps, fr_mm_s);
    return point_steps;
}

static bool measure_origin_multipoint(AxisEnum axis, const xy_long_t &origin_steps,
    xy_pos_t &origin, xy_pos_t &distance, const float fr_mm_s) {
    // scramble probing sequence to improve belt redistribution when estimating the centroid
    // unit is full AB cycles away from homing corner as given to plan_corexy_abgrid_move()
    static constexpr xy_long_t point_sequence[] = {
        { 1, 0 },
        { -1, 0 },
        { 0, 1 },
        { 0, -1 },
        { -1, -1 },
        { 1, 1 },
        { 1, -1 },
        { -1, 1 },
        { 0, 0 },
    };

    struct point_data {
        xy_pos_t c_dist;
        xy_pos_t m_dist;
        bool revalidate;
    };

    point_data points[std::size(point_sequence)];

    // allow single-point revalidation on instability to speed-up retries
    // start by forcing whole-grid revalidation
    for (size_t i = 0; i != std::size(point_sequence); ++i) {
        points[i].revalidate = true;
    }

    // keep track of points to revalidate
    size_t rev_cnt = std::size(point_sequence);
    for (size_t revcount = 0; revcount < std::size(point_sequence) / 2; ++revcount) {
        xy_pos_t c_acc = { 0, 0 };
        xy_pos_t m_acc = { 0, 0 };
        size_t new_rev_cnt = 0;

        // cycle through grid points and calculate centroid
        for (size_t i = 0; i != std::size(point_sequence); ++i) {
            const auto &seq = point_sequence[i];
            auto &data = points[i];

            if (data.revalidate) {
                plan_corexy_abgrid_move(origin_steps, seq, fr_mm_s);
                if (planner.draining()) {
                    return false;
                }

                if (!measure_phase_cycles(axis, data.c_dist, data.m_dist)) {
                    return false;
                }
            }

            c_acc += data.c_dist;
            m_acc += data.m_dist;
        }
        origin = c_acc / float(std::size(point_sequence));
        distance = m_acc / float(std::size(point_sequence));

        metric_record_custom(&metric_phxy_orig, ",a=%u,t=\"s\" c=%u,p=%u,i=%u,r=%u",
            axis, internal::cal_id, internal::probe_id, revcount, (unsigned)rev_cnt);
        metric_record_custom(&metric_phxy_orig, ",a=%u,t=\"o\" c=%u,o0=%.3f,o1=%.3f",
            axis, internal::cal_id, (double)origin[0], (double)origin[1]);
        metric_record_custom(&metric_phxy_orig, ",a=%u,t=\"d\" c=%u,d0=%.3f,d1=%.3f",
            axis, internal::cal_id, (double)distance[0], (double)distance[1]);

        // verify each probed point with the current centroid
        xy_long_t o_int = { long(roundf(origin[A_AXIS])), long(roundf(origin[B_AXIS])) };
        for (size_t i = 0; i != std::size(point_sequence); ++i) {
            const auto &seq = point_sequence[i];
            auto &data = points[i];

            const xy_long_t c_ab = cdist_translate(data.c_dist, origin);
            const xy_long_t c_diff = c_ab - seq - o_int;
            const bool c_unstable = point_is_unstable(data.c_dist, origin);
            const bool c_invalid = c_diff[A_AXIS] || c_diff[B_AXIS];

            metric_record_custom(&metric_phxy_orig, ",a=%u,t=\"p\" c=%u,p=%u,c0=%li,c1=%li,s=%u",
                axis, internal::cal_id, i, (long)c_ab[0], (long)c_ab[1], c_unstable);
            metric_record_custom(&metric_phxy_orig, ",a=%u,t=\"p\" c=%u,p=%u,d0=%li,d1=%li,v=%u",
                axis, internal::cal_id, i, (long)c_diff[0], (long)c_diff[1], c_invalid);
            idle(true, true); // allow some time to flush the metrics buffer

            if (c_invalid) {
                internal::home_unstable = true;
                SERIAL_ECHOLNPAIR("home calibration point (", seq[A_AXIS], ",", seq[B_AXIS],
                    ") invalid A:", c_diff[A_AXIS], " B:", c_diff[B_AXIS],
                    " with origin A:", o_int[A_AXIS], " B:", o_int[B_AXIS]);
                // when even just a point is invalid, we likely have skipped or have a false centroid:
                // no point in revalidating, mark the calibration as an instant failure
                return false;
            }

            data.revalidate = c_unstable;
            if (data.revalidate) {
                internal::home_unstable = true;
                SERIAL_ECHOLNPAIR("home calibration point (", seq[A_AXIS], ",", seq[B_AXIS],
                    ") unstable A:", data.c_dist[A_AXIS], " B:", data.c_dist[B_AXIS],
                    " with origin A:", origin[A_AXIS], " B:", origin[B_AXIS]);
                ++new_rev_cnt;
            }
        }

        if (new_rev_cnt > rev_cnt) {
            // we got worse, likely we have moved the centroid: give up
            return false;
        }
        rev_cnt = new_rev_cnt;
        if (!rev_cnt) {
            // stop if all points are valid
            break;
        }
    }
    if (rev_cnt) {
        // we left with unstable points, reject calibration
        return false;
    }

    SERIAL_ECHOLNPAIR("home grid origin A:", origin[A_AXIS], " B:", origin[B_AXIS]);
    return true;
}

bool corexy_rehome_xy(float fr_mm_s) {
    // enable endstops locally
    const bool endstops_enabled = endstops.is_enabled();
    ScopeGuard endstop_restorer([&]() {
        endstops.enable(endstops_enabled);
    });
    endstops.enable(true);

    if (ENABLED(HOME_Y_BEFORE_X)) {
        if (!homeaxis(Y_AXIS, fr_mm_s, false, nullptr, false)) {
            return false;
        }
    }
    if (!homeaxis(X_AXIS, fr_mm_s, false, nullptr, false)) {
        return false;
    }
    if (DISABLED(HOME_Y_BEFORE_X)) {
        if (!homeaxis(Y_AXIS, fr_mm_s, false, nullptr, false)) {
            return false;
        }
    }
    return true;
}

/**
 * @brief Rehome, move into position, align to phase and return current position
 * @param origin_pos Final/current home position
 * @param origin_steps Final/current stepper position
 * @param fr_mm_s Service move feedrate
 * @param rehome If true, also perform initial home
 */
static bool corexy_rehome_and_phase(xyze_pos_t &origin_pos, xy_long_t &origin_steps, float fr_mm_s, bool rehome) {
    // ignore starting position if requested, otherwise assume to be already homed
    if (rehome) {
        corexy_rehome_xy(fr_mm_s);
    }

    // reposition parallel to the origin
    origin_pos = current_position;
    origin_pos[X_AXIS] = (base_home_pos(X_AXIS) - XY_HOMING_ORIGIN_OFFSET * X_HOME_DIR);
    origin_pos[Y_AXIS] = (base_home_pos(Y_AXIS) - XY_HOMING_ORIGIN_OFFSET * Y_HOME_DIR);
    planner.buffer_line(origin_pos, fr_mm_s, active_extruder);
    planner.synchronize();

    // align both motors to a full phase
    stepper_wait_for_standstill(_BV(A_AXIS) | _BV(B_AXIS));
    origin_steps[A_AXIS] = (stepper.position(A_AXIS) + phase_backoff_steps(A_AXIS));
    origin_steps[B_AXIS] = (stepper.position(B_AXIS) + phase_backoff_steps(B_AXIS));

    // sanity checks: don't remove these! Issues in repositioning are a result of planner/stepper
    // calculation issues which will show up elsewhere and are NOT just mechanical issues. We need
    // step-accuracy while homing! ask @wavexx when in doubt regarding these
    plan_corexy_raw_move(origin_steps, fr_mm_s);
    const xy_long_t raw_move_diff = {
        stepper.position(A_AXIS) - origin_steps[A_AXIS],
        stepper.position(B_AXIS) - origin_steps[B_AXIS]
    };
    if (raw_move_diff[A_AXIS] != 0 || raw_move_diff[B_AXIS] != 0) {
        if (planner.draining()) {
            return false;
        }
        SERIAL_ECHOLN("raw move failed");
        SERIAL_ECHOLNPAIR("diff A:", raw_move_diff[A_AXIS], " B:", raw_move_diff[B_AXIS]);
        bsod("raw move didn't reach requested position");
    }

    stepper_wait_for_standstill(_BV(A_AXIS) | _BV(B_AXIS));
    if (!phase_aligned(A_AXIS) || !phase_aligned(B_AXIS)) {
        if (planner.draining()) {
            return false;
        }
        SERIAL_ECHOLN("phase alignment failed");
        SERIAL_ECHOLNPAIR("phase A:", axis_mscnt(A_AXIS), " B:", axis_mscnt(B_AXIS));
        bsod("phase alignment failed");
    }

    return true;
}

#if HAS_TRINAMIC && defined(XY_HOMING_MEASURE_SENS_MIN)
static bool measure_calibrate_walk(float &score, AxisEnum measured_axis,
    const xy_long_t origin_steps, const float fr_mm_s, const measure_axis_params &params) {
    // prepare for repeated measurements
    const AxisEnum other_axis = (measured_axis == B_AXIS ? A_AXIS : B_AXIS);
    SetupForMeasurement setup_guard(other_axis);
    ++internal::probe_id;

    // calculate maximum reliable number of cycles to move towards the endstop
    constexpr AxisEnum walk_axis = X_HOME_DIR == Y_HOME_DIR ? X_AXIS : Y_AXIS;
    constexpr float walk_dist = XY_HOMING_ORIGIN_OFFSET - axis_home_max_diff(walk_axis) * 2;
    static_assert(walk_dist >= 0);
    const size_t walk_cycles = floor(walk_dist / (phase_cycle_steps(walk_axis) * planner.mm_per_step[walk_axis] * float(M_SQRT2)));
    const size_t walk_period = walk_cycles * 2;
    const size_t measure_probes = std::max<size_t>(walk_period, XY_HOMING_ORIGIN_BUMP_RETRIES * 2);
    assert(measure_probes >= 3 && measure_probes < 128);

    // absolute measure limit distances
    static_assert(XY_HOMING_ORIGIN_OFFSET > axis_home_max_diff(walk_axis) * 2);
    constexpr AxisEnum walk_ortho_axis = walk_axis == X_AXIS ? Y_AXIS : X_AXIS;
    constexpr int32_t measure_min_dist_mm = (XY_HOMING_ORIGIN_OFFSET - axis_home_max_diff(walk_ortho_axis) * 2) * float(M_SQRT2);
    constexpr int32_t measure_max_dist_mm = (XY_HOMING_ORIGIN_OFFSET * 4);
    const int32_t measure_min_dist = measure_min_dist_mm / planner.mm_per_step[measured_axis];
    const int32_t measure_max_dist = measure_max_dist_mm / planner.mm_per_step[measured_axis];
    const int32_t measure_dir = (measured_axis == B_AXIS ? -X_HOME_DIR : -Y_HOME_DIR);

    score = 0.f;
    constexpr float score_exp = 3.f;
    float score_acc = 0;

    for (int32_t a_dir = 1; a_dir >= -1; a_dir -= 2) {
        // start from the lowest possible probing position, then zig-zag along the centerpoint to
        // test for weaker positions in the holding rotor and take those into account in addition to
        // testing repeatibility
        int32_t p_steps_buf[measure_probes];
        size_t p_steps_cnt = 0;
        float p_dist;
        int32_t p_steps;

        for (size_t probe = 0; probe != measure_probes; ++probe) {
            const long cycle = probe / walk_period + (a_dir >= 0 ? 0 : 1);
            const long n = probe % walk_period;
            const long d = -long(walk_cycles) + (cycle % 2 ? walk_period - n : n);

            const xy_long_t temp_origin = plan_corexy_abgrid_move(origin_steps, { d * a_dir, d }, fr_mm_s);
            if (planner.draining()) {
                return false;
            }

            const bool valid = measure_axis_distance(measured_axis, temp_origin,
                measure_max_dist * measure_dir * a_dir, p_steps, p_dist, params);
            if (!valid) {
                if (planner.draining()) {
                    return false;
                }
            } else {
                p_steps = abs(p_steps);
                if (p_steps >= measure_min_dist && p_steps <= measure_max_dist) {
                    p_steps_buf[p_steps_cnt++] = p_steps;
                }
            }
        }

        if (p_steps_cnt >= 3) {
            // calculate a score based on central phase deviation independently per-direction
            std::sort(&p_steps_buf[0], &p_steps_buf[p_steps_cnt]);
            const int32_t steps_med = p_steps_buf[p_steps_cnt / 2];
            for (size_t i = 0; i != p_steps_cnt; ++i) {
                const int32_t p_off = abs(steps_med - p_steps_buf[i]) * 4 / phase_cycle_steps(measured_axis);
                const float p_score = 1.f / std::pow(1.f + p_off, score_exp);
                score_acc += p_score;
            }
        }
    }

    // normalize the score by absolute maximum
    score = score_acc / (measure_probes * 2 * std::pow(2.f, score_exp));

    return true;
}

static bool measure_calibrate_sens(CoreXYHomeTMCSens &calibrated_sens,
    AxisEnum measured_axis, const float fr_mm_s) {
    measure_axis_params params = measure_axis_defaults(measured_axis);
    bool rehome = false; // initial home state

    // limits are inclusive
    static_assert(XY_HOMING_MEASURE_SENS_MAX > XY_HOMING_MEASURE_SENS_MIN);
    constexpr size_t slots = (XY_HOMING_MEASURE_SENS_MAX - XY_HOMING_MEASURE_SENS_MIN) + 1;
    static_assert(slots > 1 && slots < 16);
    std::pair<int8_t, float> scores[slots];
    size_t score_cnt = 0;

    for (int8_t sens = XY_HOMING_MEASURE_SENS_MIN; sens <= XY_HOMING_MEASURE_SENS_MAX; ++sens) {
        xyze_pos_t origin_pos;
        xy_long_t origin_steps;

        // reposition parallel to the origin to our probing point
        if (!corexy_rehome_and_phase(origin_pos, origin_steps, fr_mm_s, rehome)) {
            return false;
        }

        // adjust sensitivity and calculate score
        params.sensitivity = sens;
        float score;
        if (!measure_calibrate_walk(score, measured_axis, origin_steps, fr_mm_s, params)) {
            if (planner.draining()) {
                return false;
            }
        } else {
            scores[score_cnt].first = sens;
            scores[score_cnt].second = score;
            ++score_cnt;

            metric_record_custom(&metric_phxy_sens, ",a=%u,sens=%i s=%.4f,f=%.2f,c=%u",
                measured_axis, sens, (double)score, (double)params.feedrate, params.current);
        }

        // always rehome to ignore any undetected skip
        rehome = true;
    }
    if (!score_cnt) {
        return false;
    }

    // pick the best result
    size_t best_idx = 0;
    SERIAL_ECHOLN("sensitivity calibration");
    for (size_t i = 0; i != score_cnt; ++i) {
        SERIAL_ECHOLNPAIR(" sens:", scores[i].first, " score:", scores[i].second);
        if (scores[i].second > scores[best_idx].second) {
            best_idx = i;
        }
    }
    SERIAL_ECHOLNPAIR(" selected:", scores[best_idx].first);

    // we currently only calibrate sensitivity, but save all effective parameters
    calibrated_sens.feedrate = params.feedrate;
    calibrated_sens.current = params.current;
    calibrated_sens.sensitivity = scores[best_idx].first;
    calibrated_sens.score = scores[best_idx].second;
    return true;
}

bool corexy_sens_calibrate(const float fr_mm_s) {
    const AxisEnum measured_axis = (X_HOME_DIR == Y_HOME_DIR ? B_AXIS : A_AXIS);

    // finish previous moves and disable main endstop/crash recovery handling
    planner.synchronize();
    #if ENABLED(CRASH_RECOVERY)
    crash_s.not_for_replay();
    Crash_Temporary_Deactivate ctd;
    #endif /*ENABLED(CRASH_RECOVERY)*/

    // disable endstops locally
    const bool endstops_enabled = endstops.is_enabled();
    ScopeGuard endstop_restorer([&]() {
        endstops.enable(endstops_enabled);
    });
    endstops.not_homing();

    SERIAL_ECHOLN("recalibrating homing sensitivity");
    ui.status_printf_P(0, "Recalibrating home. Printer may vibrate and be noisier.");

    CoreXYHomeTMCSens calibrated_sens;
    if (!measure_calibrate_sens(calibrated_sens, measured_axis, fr_mm_s)) {
        SERIAL_ECHOLNPAIR("home sensitivity calibration failed");
        return false;
    }

    config_store().corexy_home_tmc_sens.set(calibrated_sens);
    return true;
}

bool corexy_sens_is_calibrated() {
    const CoreXYHomeTMCSens calibrated_sens = config_store().corexy_home_tmc_sens.get();
    return !calibrated_sens.uninitialized();
}
#endif

// Refine home origin precisely on core-XY.
bool corexy_home_refine(float fr_mm_s, CoreXYCalibrationMode mode) {
    const AxisEnum measured_axis = (X_HOME_DIR == Y_HOME_DIR ? B_AXIS : A_AXIS);

    // finish previous moves and disable main endstop/crash recovery handling
    planner.synchronize();
#if ENABLED(CRASH_RECOVERY)
    crash_s.not_for_replay();
    Crash_Temporary_Deactivate ctd;
#endif /*ENABLED(CRASH_RECOVERY)*/

    // disable endstops locally
    const bool endstops_enabled = endstops.is_enabled();
    ScopeGuard endstop_restorer([&]() {
        endstops.enable(endstops_enabled);
    });
    endstops.not_homing();

    // reset previous home state
    internal::home_unstable = false;
    ++internal::refine_id;

    // reposition parallel to the origin to our probing point
    xyze_pos_t origin_pos;
    xy_long_t origin_steps;
    if (!corexy_rehome_and_phase(origin_pos, origin_steps, fr_mm_s, false)) {
        return false;
    }

    // calibrate origin if not done already
    CoreXYGridOrigin calibrated_origin = config_store().corexy_grid_origin.get();
    if ((mode == CoreXYCalibrationMode::force)
        || ((mode == CoreXYCalibrationMode::on_demand) && calibrated_origin.uninitialized())) {
        SERIAL_ECHOLN("recalibrating home origin");
        ui.status_printf_P(0, "Recalibrating home. Printer may vibrate and be noisier.");
        ++internal::cal_id;

        xy_pos_t origin, distance;
        if (!measure_origin_multipoint(measured_axis, origin_steps, origin, distance, fr_mm_s)) {
            SERIAL_ECHOLNPAIR("home origin calibration failed");
            return false;
        }

        LOOP_XY(axis) {
            calibrated_origin.origin[axis] = origin[axis];
            calibrated_origin.distance[axis] = distance[axis];
        }
        config_store().corexy_grid_origin.set(calibrated_origin);
    } else if (calibrated_origin.uninitialized()) {
        // we have no origin, but calibration was explicitly disabled: continue without
        SERIAL_ECHOLN("homing without calibrated origin");
        calibrated_origin.origin[A_AXIS] = 0.f;
        calibrated_origin.origin[B_AXIS] = 0.f;
    }
    const xy_pos_t calibrated_origin_xy = {
        calibrated_origin.origin[A_AXIS],
        calibrated_origin.origin[B_AXIS]
    };
    metric_record_custom(&metric_phxy_home, ",t=\"o\" h=%u,o0=%.2f,o1=%.3f",
        internal::refine_id, (double)calibrated_origin_xy[0], (double)calibrated_origin_xy[1]);

    // measure from current origin
    xy_pos_t c_dist, _;
    if (!measure_phase_cycles(measured_axis, c_dist, _)) {
        return false;
    }

    // validate current origin
    const bool home_unstable = point_is_unstable(c_dist, calibrated_origin_xy);
    if (home_unstable) {
        SERIAL_ECHOLNPAIR("home point is unstable");
    }

    const xy_long_t c_ab = cdist_translate(c_dist, calibrated_origin_xy);
    metric_record_custom(&metric_phxy_home, ",t=\"h\" h=%u,c0=%.3f,c1=%.3f,s=%u",
        internal::refine_id, (double)c_ab[0], (double)c_ab[1], home_unstable);

    // validate from another point in the AB grid
    const xy_long_t v_ab_off = { -1, 3 };
    plan_corexy_abgrid_move(origin_steps, v_ab_off, fr_mm_s);
    if (planner.draining()) {
        return false;
    }
    xy_pos_t v_c_dist;
    if (!measure_phase_cycles(measured_axis, v_c_dist, _)) {
        return false;
    }

    const xy_long_t v_c_ab = cdist_translate(v_c_dist, calibrated_origin_xy);
    const xy_long_t v_c_diff = v_c_ab - v_ab_off;
    const bool v_home_unstable = point_is_unstable(v_c_dist, calibrated_origin_xy);
    const bool v_c_invalid = v_c_diff != c_ab;

    metric_record_custom(&metric_phxy_home, ",t=\"v\" h=%u,c0=%.3f,c1=%.3f,s=%u,v=%u",
        internal::refine_id, (double)v_c_ab[0], (double)v_c_ab[1], v_home_unstable, v_c_invalid);

    if (v_c_invalid) {
        internal::home_unstable = true;
        SERIAL_ECHOLNPAIR("home validation point is invalid");
        return false;
    }
    if (v_home_unstable) {
        SERIAL_ECHOLNPAIR("home validation point is unstable");
    }
    if (home_unstable && v_home_unstable) {
        // mark home as unstable only if both points are
        internal::home_unstable = true;
    }

    // move back to origin
    plan_corexy_raw_move(origin_steps, fr_mm_s);
    if (planner.draining()) {
        return false;
    }

    // set machine origin
    const xy_long_t c_ab_steps = {
        c_ab[X_HOME_DIR == Y_HOME_DIR ? A_AXIS : B_AXIS] * phase_cycle_steps(A_AXIS) * -Y_HOME_DIR,
        c_ab[X_HOME_DIR == Y_HOME_DIR ? B_AXIS : A_AXIS] * phase_cycle_steps(B_AXIS) * -X_HOME_DIR
    };
    xy_pos_t c_mm;
    corexy_ab_to_xy(c_ab_steps, c_mm);
    current_position.x = c_mm[X_AXIS] + origin_pos[X_AXIS] + XY_HOMING_ORIGIN_OFFSET * X_HOME_DIR;
    current_position.y = c_mm[Y_AXIS] + origin_pos[Y_AXIS] + XY_HOMING_ORIGIN_OFFSET * Y_HOME_DIR;
    planner.set_machine_position_mm(current_position);

    SERIAL_ECHOLNPAIR("calibrated home cycle A:", c_ab[A_AXIS], " B:", c_ab[B_AXIS]);
    return true;
}

bool corexy_home_is_calibrated() {
    const CoreXYGridOrigin calibrated_origin = config_store().corexy_grid_origin.get();
    return !calibrated_origin.uninitialized();
}

bool corexy_home_is_unstable() {
    const CoreXYGridOrigin calibrated_origin = config_store().corexy_grid_origin.get();
    return calibrated_origin.uninitialized() || internal::home_unstable;
}
